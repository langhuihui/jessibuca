import Player from './player';
import Events from "./utils/events";
import {DEMUX_TYPE, EVENTS, PLAYER_PLAY_PROTOCOL} from "./constant";


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

    _createPlayer(option) {
        const player = new Player(this.$container, option);

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

    //
    play(url) {
        return new Promise((resolve, reject) => {

            if (!url) {
                reject();
            }
            //
            if (this.player) {
                this.player.destroy();
                this.player = null;
            }
            //
            const isHttp = url.indexOf("http") === 0;
            //
            const protocol = isHttp ? PLAYER_PLAY_PROTOCOL.fetch : PLAYER_PLAY_PROTOCOL.websocket
            //
            const demuxType = (isHttp || url.indexOf(".flv") !== -1 || this._opt.isFlv) ? DEMUX_TYPE.flv : DEMUX_TYPE.m7s;

            const options = {
                ...this._opt,
                protocol,
                demuxType,
                url
            }

            this.player = this._createPlayer(options);

            if (this.player.hasLoaded()) {
                this.player.play().then(() => {
                    resolve();
                }).catch(() => {
                    this.player.destroy();
                    this.player = null;
                    reject();
                })
            } else {
                this.player.once(EVENTS.load, () => {
                    this.player.play().then(() => {
                        resolve();
                    }).catch(() => {
                        this.player.destroy();
                        this.player = null;
                        reject();
                    })
                })
            }
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
