import Player from './player';
import Events from "./utils/events";


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

        this._opt = _opt;
        this.$container = $container;
        this.player = null;
        this.href = null;
        this.events = new Events(this);
    }

    _createPlayer($container, option) {
        const player = new Player($container, option);

        return player;
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

    play(url) {
        return new Promise((resolve, reject) => {

            if (!url) {
                reject();
            }
            if (this.player) {
                this.player.destroy();
                this.player = null;
            }



            const options = {
                ...this._opt,

            }

            this.player = this._createPlayer()
        })
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
