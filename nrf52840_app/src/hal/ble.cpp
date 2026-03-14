#include "ble.h"
#include "../hal/rtc.h"
#include "../app/app.h"
#include "../app/events.h"
#include <bluefruit.h>
#include <clients/BLEAncs.h>
#include <string.h>
#include <stdlib.h>

namespace
{
    BLEHidAdafruit s_hid;
    BLEUart        s_uart;
    BLEAncs        s_ancs;

    // Persistent state exposed via getters
    NotifState    s_notif;
    MessagesState s_messages;
    ThreadState   s_thread;

    // MusicState uses const char* — back them with static buffers
    char       s_music_title[64]  = "No Title";
    char       s_music_artist[64] = "Unknown";
    MusicState s_music;  // default: title="No Title", artist="Unknown" (string literals)

    // Line accumulation buffer (NUS data can arrive in chunks)
    constexpr int LINE_BUF = 256;
    char s_line[LINE_BUF];
    int  s_line_len = 0;

    // Protocol handlers

    // T|HH|MM|SS|YYYY|MM|DD|wday
    void handle_T(char* rest)
    {
        rtc::DateTime dt{};
        char* tok = strtok(rest, "|");
        if (!tok) return; 
        dt.hour  = (uint8_t)atoi(tok);
        tok = strtok(nullptr, "|");

        if (!tok) return; 
        dt.min = (uint8_t)atoi(tok);
        tok = strtok(nullptr, "|");

        if (!tok) return;
        dt.sec = (uint8_t)atoi(tok);
        tok = strtok(nullptr, "|");

        if (!tok) return;
        dt.year  = (uint16_t)atoi(tok);
        tok = strtok(nullptr, "|");

        if (!tok) return;
        dt.month = (uint8_t)atoi(tok);
        tok = strtok(nullptr, "|");

        if (!tok) return;
        dt.day = (uint8_t)atoi(tok);
        tok = strtok(nullptr, "|");
        
        if (!tok) return;
        dt.wday = (uint8_t)atoi(tok);

        rtc::set(dt);
        app::post(Event(EventType::BLE_TIME_SYNC));
    }

    // N|appid|title|body
    void handle_N(char* rest)
    {
        if (s_notif.count >= 8) return;

        char* tok = strtok(rest, "|");
        if (!tok) return;
        uint8_t appid = (uint8_t)atoi(tok);

        tok = strtok(nullptr, "|");
        if (!tok) return;
        const char* title = tok;

        tok = strtok(nullptr, "|");
        const char* body = tok ? tok : "";

        NotifEntry& entry = s_notif.entries[s_notif.count];
        entry.source = (AppId)(appid < (uint8_t)AppId::COUNT ? appid : 0);
        strncpy(entry.title, title, sizeof(entry.title) - 1);
        entry.title[sizeof(entry.title) - 1] = '\0';
        strncpy(entry.body, body, sizeof(entry.body) - 1);
        entry.body[sizeof(entry.body) - 1] = '\0';
        s_notif.count++;

        app::post(Event(EventType::BLE_NOTIF));
    }

    // NC  (no fields)
    void handle_NC()
    {
        s_notif.count = 0;
        app::post(Event(EventType::BLE_NOTIF));
    }

    // MC|title|artist|elapsed|duration|playing
    void handle_MC(char* rest)
    {
        char* tok = strtok(rest, "|");
        if (!tok) return;
        strncpy(s_music_title, tok, sizeof(s_music_title) - 1);
        s_music_title[sizeof(s_music_title) - 1] = '\0';

        tok = strtok(nullptr, "|");
        if (!tok) return;
        strncpy(s_music_artist, tok, sizeof(s_music_artist) - 1);
        s_music_artist[sizeof(s_music_artist) - 1] = '\0';

        tok = strtok(nullptr, "|");
        if (tok) s_music.elapsed  = (uint32_t)atol(tok);
        tok = strtok(nullptr, "|");
        if (tok) s_music.duration = (uint32_t)atol(tok);
        tok = strtok(nullptr, "|");
        if (tok) s_music.playing  = (atoi(tok) != 0);

        s_music.title  = s_music_title;
        s_music.artist = s_music_artist;

        app::post(Event(EventType::BLE_MUSIC_META));
    }

    // MS|elapsed|duration
    void handle_MS(char* rest)
    {
        char* tok = strtok(rest, "|");
        if (tok) s_music.elapsed  = (uint32_t)atol(tok);
        tok = strtok(nullptr, "|");
        if (tok) s_music.duration = (uint32_t)atol(tok);

        app::post(Event(EventType::BLE_MEDIA_STATE,
                        (int32_t)s_music.elapsed,
                        (int32_t)s_music.duration));
    }

    // MG|peer|preview|unread
    void handle_MG(char* rest)
    {
        char* tok = strtok(rest, "|");
        if (!tok) return;
        const char* peer = tok;

        tok = strtok(nullptr, "|");
        const char* preview = tok ? tok : "";

        tok = strtok(nullptr, "|");
        bool unread = tok ? (atoi(tok) != 0) : false;

        // Update existing conversation if peer already in list
        for (int i = 0; i < s_messages.count; i++)
        {
            if (strncmp(s_messages.convs[i].peer, peer, sizeof(Conversation::peer)) == 0)
            {
                strncpy(s_messages.convs[i].preview, preview, sizeof(Conversation::preview) - 1);
                s_messages.convs[i].preview[sizeof(Conversation::preview) - 1] = '\0';
                s_messages.convs[i].unread = unread;
                app::post(Event(EventType::BLE_MSG_UPDATE));
                return;
            }
        }

        if (s_messages.count >= 8) return;
        Conversation& c = s_messages.convs[s_messages.count++];
        strncpy(c.peer,    peer,    sizeof(c.peer)    - 1); c.peer[sizeof(c.peer)-1]       = '\0';
        strncpy(c.preview, preview, sizeof(c.preview) - 1); c.preview[sizeof(c.preview)-1] = '\0';
        c.unread = unread;

        app::post(Event(EventType::BLE_MSG_UPDATE));
    }

    // MGC  (no fields)
    void handle_MGC()
    {
        s_messages.count = 0;
        app::post(Event(EventType::BLE_MSG_UPDATE));
    }

    // MT|peer|sent|text  — add one message to a thread (switches active thread if peer differs)
    void handle_MT(char* rest)
    {
        char* tok = strtok(rest, "|");
        if (!tok) return;
        const char* peer = tok;

        tok = strtok(nullptr, "|");
        if (!tok) return;
        bool sent = (atoi(tok) != 0);

        tok = strtok(nullptr, "|");
        const char* text = tok ? tok : "";

        // Reset thread if switching to a different peer
        if (strncmp(s_thread.peer, peer, sizeof(s_thread.peer)) != 0)
        {
            s_thread.count = 0;
            strncpy(s_thread.peer, peer, sizeof(s_thread.peer) - 1);
            s_thread.peer[sizeof(s_thread.peer) - 1] = '\0';
        }

        if (s_thread.count >= 32) return;
        ThreadMessage& m = s_thread.messages[s_thread.count++];
        strncpy(m.text, text, sizeof(m.text) - 1);
        m.text[sizeof(m.text) - 1] = '\0';
        m.sent = sent;

        app::post(Event(EventType::BLE_THREAD_UPDATE));
    }

    // MTC  (no fields) — clear thread
    void handle_MTC()
    {
        s_thread.count = 0;
        app::post(Event(EventType::BLE_THREAD_UPDATE));
    }

    // Dispatch one complete \n-terminated line
    void dispatch_line(char* line)
    {
        char* sep  = strchr(line, '|');
        char* rest = sep ? sep + 1 : nullptr;
        if (sep) *sep = '\0';

        if      (strcmp(line, "T")   == 0 && rest) handle_T(rest);
        else if (strcmp(line, "N")   == 0 && rest) handle_N(rest);
        else if (strcmp(line, "NC")  == 0)         handle_NC();
        else if (strcmp(line, "MC")  == 0 && rest) handle_MC(rest);
        else if (strcmp(line, "MS")  == 0 && rest) handle_MS(rest);
        else if (strcmp(line, "MG")  == 0 && rest) handle_MG(rest);
        else if (strcmp(line, "MGC") == 0)         handle_MGC();
        else if (strcmp(line, "MT")  == 0 && rest) handle_MT(rest);
        else if (strcmp(line, "MTC") == 0)         handle_MTC();
    }

    // ANCS

    // Queue ANCS notification UIDs so attribute fetching (which blocks) happens
    // in ble::poll() on the main loop, not inside the BLE event callback.
    struct AncsEntry { uint32_t uid; uint8_t categoryID; };
    AncsEntry        s_ancs_queue[8];
    volatile int     s_ancs_q_head = 0;
    volatile int     s_ancs_q_tail = 0;

    // Maps an ANCS category ID + iOS bundle ID fragment to an AppId
    AppId ancs_map_source(uint8_t cat, const char* appID)
    {
        if (cat == ANCS_CAT_INCOMING_CALL || cat == ANCS_CAT_MISSED_CALL ||
            cat == ANCS_CAT_SOCIAL        ||
            strstr(appID, "mobilesms")    || strstr(appID, "MobilePhone") ||
            strstr(appID, "Telegram")     || strstr(appID, "WhatsApp"))
            return AppId::MESSAGES;
        if (cat == ANCS_CAT_ENTERTAINMENT || strstr(appID, "Music") || strstr(appID, "Spotify"))
            return AppId::MUSIC;
        return AppId::SETTINGS; // generic
    }

    // Called from BLE event task — only enqueues UID, no blocking calls
    void ancs_notification_cb(AncsNotification_t* notif)
    {
        if (notif->eventID != ANCS_EVT_NOTIFICATION_ADDED) return;
        if (notif->eventFlags.preExisting) return; // skip pre-existing on connect
        int next = (s_ancs_q_head + 1) % 8;
        if (next != s_ancs_q_tail)
        {
            s_ancs_queue[s_ancs_q_head] = { notif->uid, notif->categoryID };
            s_ancs_q_head = next;
        }
    }

    // Called when iPhone connects — discover ANCS (requires bonding)
    void connect_callback(uint16_t conn_handle)
    {
        s_ancs.discover(conn_handle);
    }

    // Called after pairing/encryption is established — enable ANCS notifications
    void secured_callback(uint16_t conn_handle)
    {
        if (s_ancs.discovered())
            s_ancs.enableNotification();
    }

    // NUS RX callback — BLEUart::rx_callback_t is void(*)(uint16_t conn_handle)
    // Read available bytes from s_uart and accumulate into lines.
    // Flushes any pending line at end of packet so commands without a trailing
    // newline (e.g. written from nRF Connect) are still dispatched.
    void rx_callback(uint16_t /*conn_handle*/)
    {
        while (s_uart.available())
        {
            char c = (char)s_uart.read();
            if (c == '\n' || c == '\r')
            {
                if (s_line_len > 0)
                {
                    s_line[s_line_len] = '\0';
                    dispatch_line(s_line);
                    s_line_len = 0;
                }
            }
            else if (s_line_len < LINE_BUF - 1)
            {
                s_line[s_line_len++] = c;
            }
            // overflow silently dropped
        }

        // Flush partial line — handles writes that don't include a trailing newline
        if (s_line_len > 0)
        {
            s_line[s_line_len] = '\0';
            dispatch_line(s_line);
            s_line_len = 0;
        }
    }
}

namespace ble
{
    void init()
    {
        Bluefruit.begin();
        Bluefruit.setTxPower(4);
        Bluefruit.setName("SmartWatch");

        // HID peripheral (media keys)
        s_hid.begin();

        // NUS (Nordic UART Service) — phone companion app writes commands here
        s_uart.begin();
        s_uart.setRxCallback(rx_callback);

        // ANCS client — receives iOS notifications
        s_ancs.begin();
        s_ancs.setNotificationCallback(ancs_notification_cb);

        // Pairing: Just Works (no display/keyboard needed)
        Bluefruit.Security.setIOCaps(BLE_GAP_IO_CAPS_NONE);

        // Callbacks for ANCS — must be secured before ANCS notifications flow
        Bluefruit.setConnectCallback(connect_callback);
        Bluefruit.Security.setSecuredCallback(secured_callback);

        // Advertising packet (~28 bytes):
        //   flags + txpower + HID appearance + ANCS solicitation UUID
        // iOS looks for the ANCS solicitation UUID to know this device wants notifications.
        Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
        Bluefruit.Advertising.addTxPower();
        Bluefruit.Advertising.addAppearance(BLE_APPEARANCE_GENERIC_HID);
        Bluefruit.Advertising.addService(s_ancs);  // ANCS solicitation UUID

        // Scan Response (~30 bytes):
        //   NUS service UUID (iOS companion app scans for this) + device name
        Bluefruit.ScanResponse.addService(s_uart);
        Bluefruit.ScanResponse.addName();

        Bluefruit.Advertising.restartOnDisconnect(true);
        Bluefruit.Advertising.start(0);
    }

    void media_prev()
    {
        s_hid.consumerKeyPress(HID_USAGE_CONSUMER_SCAN_PREVIOUS);
        s_hid.consumerKeyRelease();
    }

    void media_playpause()
    {
        s_hid.consumerKeyPress(HID_USAGE_CONSUMER_PLAY_PAUSE);
        s_hid.consumerKeyRelease();
    }

    void media_next()
    {
        s_hid.consumerKeyPress(HID_USAGE_CONSUMER_SCAN_NEXT);
        s_hid.consumerKeyRelease();
    }

    const NotifState&    notif_state()    { return s_notif;    }
    const MusicState&    music_state()    { return s_music;    }
    const MessagesState& messages_state() { return s_messages; }
    const ThreadState&   thread_state()   { return s_thread;   }

    // Called from app::loop() — fetches ANCS attributes for queued notifications
    // (getTitle/getMessage block waiting for iPhone response, so can't run in BLE callback)
    void poll()
    {
        while (s_ancs_q_tail != s_ancs_q_head)
        {
            AncsEntry entry = s_ancs_queue[s_ancs_q_tail];
            s_ancs_q_tail = (s_ancs_q_tail + 1) % 8;

            if (s_notif.count >= 8) continue;

            char title[32] = {};
            char body[64]  = {};
            char appID[64] = {};

            s_ancs.getTitle(entry.uid,   title, sizeof(title));
            s_ancs.getMessage(entry.uid, body,  sizeof(body));
            s_ancs.getAppID(entry.uid,   appID, sizeof(appID));

            NotifEntry& e = s_notif.entries[s_notif.count++];
            e.source = ancs_map_source(entry.categoryID, appID);
            strncpy(e.title, title, sizeof(e.title) - 1); e.title[sizeof(e.title)-1] = '\0';
            strncpy(e.body,  body,  sizeof(e.body)  - 1); e.body[sizeof(e.body)-1]   = '\0';

            app::post(Event(EventType::BLE_NOTIF));
        }
    }
}
