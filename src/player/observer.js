export default (player) => {

    const {
        _opt,
        debug,
        events: {proxy},
    } = player;


    if (_opt.supportDblclickFullscreen) {
        proxy(player.$videoElement, 'dblclick', () => {
            player.fullscreen = !player.fullscreen;
        })
    }


    //
    proxy(document, 'visibilitychange', () => {
        if (_opt.hiddenAutoPause) {
            debug.log('visibilitychange', document.visibilityState, player._isPlayingBeforePageHidden)

            if ("visible" === document.visibilityState) {
                if (player._isPlayingBeforePageHidden) {
                    player.play();
                }
            } else {
                player._isPlayingBeforePageHidden = player.playing;
                // hidden
                if (player.playing) {
                    player.pause();
                }
            }
        }
    })

    proxy(window, 'fullscreenchange', () => {
        //
        if (player._wakeLock !== null && "visible" === document.visibilityState) {
            player.enableWakeLock();
        }
    })
}
