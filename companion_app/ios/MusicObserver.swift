import Foundation
import MediaPlayer

// Polls MPNowPlayingInfoCenter every 5 seconds and sends MC|... to the watch.
// Works for Apple Music, Spotify, YouTube Music, and any app that updates
// the system Now Playing info.
class MusicObserver {
    private weak var bleManager: BLEManager?
    private var timer: Timer?
    private var lastTitle  = ""
    private var lastArtist = ""

    init(bleManager: BLEManager) {
        self.bleManager = bleManager
    }

    func start() {
        // Fire immediately, then every 5 seconds
        checkAndSend()
        timer = Timer.scheduledTimer(withTimeInterval: 5.0, repeats: true) { [weak self] _ in
            self?.checkAndSend()
        }
        // Also respond quickly to Apple Music track changes
        let player = MPMusicPlayerController.systemMusicPlayer
        player.beginGeneratingPlaybackNotifications()
        NotificationCenter.default.addObserver(
            self,
            selector: #selector(nowPlayingChanged),
            name: .MPMusicPlayerControllerNowPlayingItemDidChange,
            object: player)
        NotificationCenter.default.addObserver(
            self,
            selector: #selector(nowPlayingChanged),
            name: .MPMusicPlayerControllerPlaybackStateDidChange,
            object: player)
    }

    func stop() {
        timer?.invalidate()
        timer = nil
        MPMusicPlayerController.systemMusicPlayer.endGeneratingPlaybackNotifications()
        NotificationCenter.default.removeObserver(self)
    }

    @objc private func nowPlayingChanged() {
        checkAndSend()
    }

    private func checkAndSend() {
        let info     = MPNowPlayingInfoCenter.default().nowPlayingInfo
        let title    = info?[MPMediaItemPropertyTitle]                    as? String ?? ""
        let artist   = info?[MPMediaItemPropertyArtist]                   as? String ?? ""
        let duration = info?[MPMediaItemPropertyPlaybackDuration]         as? Double ?? 0
        let elapsed  = info?[MPNowPlayingInfoPropertyElapsedPlaybackTime] as? Double ?? 0
        let rate     = info?[MPNowPlayingInfoPropertyPlaybackRate]        as? Double ?? 0
        let playing  = rate > 0 ? 1 : 0

        guard !title.isEmpty else { return }

        // Avoid spamming the watch with identical state every poll tick
        let isNewTrack = (title != lastTitle || artist != lastArtist)
        if isNewTrack {
            lastTitle  = title
            lastArtist = artist
        }

        // Always send on track change; throttle seek-only updates to the 5s poll cadence
        bleManager?.write("MC|\(title)|\(artist)|\(Int(elapsed))|\(Int(duration))|\(playing)")
    }

    deinit {
        stop()
    }
}
