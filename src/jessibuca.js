class Jessibuca {
    constructor(options) {
        let _opt = options;
        let $container = options.container;
        if (typeof options.container === 'string') {
            $container = document.querySelector(options.container);
        }
        if (!$container) {
            throw new Error('Jessibuca need container option');
            return;
        }

        delete _opt.container;

        this.player = this._createPlayer($container, _opt);
    }

    _createPlayer($container, option) {

    }


    setDebug() {

    }

    mute() {

    }

    cancelMute() {

    }

    audioResume() {

    }

    setTimeout() {

    }

    setScaleMode() {

    }

    pause() {

    }

    close() {

    }

    destroy() {

    }

    clearView() {

    }

    play() {

    }

    resize() {

    }

    setBufferTime() {
    }

    setRotate() {
    }

    setVolume() {

    }

    hasLoaded() {

    }

    setKeepScreenOn() {

    }

    setFullscreen() {

    }

    screenshot() {

    }

    isPlaying() {

    }

    isMute() {

    }
}
