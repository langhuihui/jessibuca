import Player from './player';
import Events from "./utils/events";
import {DEMUX_TYPE, EVENTS, JESSIBUCA_EVENTS, PLAYER_PLAY_PROTOCOL, SCALE_MODE_TYPE} from "./constant";
import {supportWCS} from "./utils";
import Emitter from "./utils/emitter";


class Jessibuca extends Emitter {
    constructor(options) {
        super()
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
        this.href = null;
        this.events = new Events(this);
        this.player = new Player($container, _opt);
    }

    /**
     *
     * @param option
     * @returns {Player}
     * @private
     */
    _createPlayer(option) {
        const player = new Player(this.$container, option);
        return player;
    }


    /**
     *
     * @param value
     */
    setDebug(value) {
        this.player.updateOption({
            isDebug: !!value
        })
    }

    /**
     *
     */
    mute() {
        this.player.audio.mute(true);
    }

    /**
     *
     */
    cancelMute() {
        this.player.audio.mute(false);
    }

    /**
     *
     */
    setVolume(value) {
        this.player.audio.setValue(value);

    }

    /**
     *
     */
    audioResume() {
        this.player.audio.audioEnabled(true);

    }

    setTimeout(value) {
        this.player.updateOption({
            timeout: Number(value)
        })
    }

    /**
     *
     * @param type: 0,1,2
     */
    setScaleMode(type) {
        type = Number(type);
        let options = {
            isFullResize: false,
            isResize: false
        }
        switch (type) {
            case SCALE_MODE_TYPE.full:
                options.isFullResize = false;
                options.isResize = false;
                break;
            case SCALE_MODE_TYPE.auto:
                options.isFullResize = false;
                options.isResize = true;
                break;
            case SCALE_MODE_TYPE.fullAuto:
                options.isFullResize = true;
                options.isResize = true;
                break;
        }

        this.player.updateOption(options);
        this.player.video.resize();
    }

    /**
     *
     */
    pause() {
        this.player.pause();
    }

    close() {
        this.player.close();
    }

    destroy() {
        this.player.destroy();
    }

    clearView() {
        this.player.video.clearView()
    }

    //
    play(url) {
        return new Promise((resolve, reject) => {

            if (!url && !this._opt.url) {
                reject();
            }


            if (this._opt.url) {

            } else {

            }


            //
            if (this.player) {
                this.player.destroy();
                this.player = null;
                // 解除绑定事件
                this.off();
            }

            this._opt.url = url;

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

            // 对外的事件
            Object.keys(JESSIBUCA_EVENTS).forEach((key) => {
                this.player.on(EVENTS[key], (value) => {
                    this.emit(key, value)
                })
            })


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
        this.player.video.resize();
    }

    setBufferTime(time) {
        time = Number(time)
        if (this.player) {

        } else {
            this._opt.videoBuffer = time
        }
    }

    setRotate(deg) {
        deg = parseInt(deg, 10)
        const list = [0, 90, 270];
        if (this._opt.rotate === deg || list.indexOf(deg) === -1) {
            return;
        }
        this.player.updateOption({
            rotate: deg
        })
        this.resize();
    }

    hasLoaded() {
        return this.player.loaded;
    }

    setKeepScreenOn() {
        this.player.updateOption({
            keepScreenOn: true
        })
    }

    useWCS() {
        this._opt.useWCS = supportWCS();
    }

    setFullscreen(flag) {
        const fullscreen = !!flag;
        if (this.player.fullscreen !== fullscreen) {
            this.player.fullscreen = fullscreen;
        }
    }

    /**
     *
     * @param filename
     * @param format
     * @param quality
     * @param type download,base64,blob
     */
    screenshot(filename, format, quality, type) {
        return this.player.video.screenshot(filename, format, quality, type)
    }

    isPlaying() {
        return this.player.playing;
    }

    /**
     * 是否静音状态
     * @returns {Boolean}
     */
    isMute() {
        return this.player.audio.isMute();
    }
}
