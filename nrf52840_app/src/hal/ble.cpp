#include "ble.h"
#include <bluefruit.h>

namespace
{
    BLEHidAdafruit s_hid;
}

namespace ble
{
    void init()
    {
        Bluefruit.begin();
        Bluefruit.setTxPower(4);
        Bluefruit.setName("SmartWatch");

        s_hid.begin();

        // Advertising
        Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
        Bluefruit.Advertising.addTxPower();
        Bluefruit.Advertising.addAppearance(BLE_APPEARANCE_GENERIC_HID);
        Bluefruit.Advertising.addService(s_hid);
        Bluefruit.ScanResponse.addName();
        Bluefruit.Advertising.restartOnDisconnect(true);
        Bluefruit.Advertising.start(0);  // 0 = advertise indefinitely
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
}
