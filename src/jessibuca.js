import Player from './player';
import Events from "./utils/events";
import {DEMUX_TYPE, EVENTS, EVENTS_ERROR, JESSIBUCA_EVENTS, PLAYER_PLAY_PROTOCOL, SCALE_MODE_TYPE} from "./constant";
import {isEmpty, isNotEmpty, supportWCS, uuid16} from "./utils";
import Emitter from "./utils/emitter";


class Jessibuca extends Emitter {
    static ERROR = EVENTS_ERROR

    static TIMEOUT = {
        loadingTimeout: EVENTS.loadingTimeout,
        delayTimeout: EVENTS.delayTimeout,
    }

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

        $container.classList.add('jessibuca-container');

        delete _opt.container;

        // s -> ms
        if (isNotEmpty(_opt.videoBuffer)) {
            _opt.videoBuffer = Number(_opt.videoBuffer) * 1000
        }

        // setting
        if (isNotEmpty(_opt.timeout)) {
            if (isEmpty(_opt.loadingTimeout)) {
                _opt.loadingTimeout = _opt.timeout;
            }

            if (isEmpty(_opt.heartTimeout)) {
                _opt.heartTimeout = _opt.timeout
            }
        }

        this._opt = _opt;
        this.$container = $container;
        this.href = null;
        this.events = new Events(this);
        this._initPlayer($container, _opt);
    }

    /**
     *
     */
    destroy() {
        this.player.destroy();
        this.player = null;
        this.off();
    }

    _initPlayer($container, options) {
        this.player = new Player($container, options);
        this._bindEvents();
    }

    _resetPlayer(options = {}) {
        this.player.destroy();
        this.player = null;
        const _options = Object.assign(this._opt, options);
        this._initPlayer(this.$container, _options);
    }

    _bindEvents() {
        // 对外的事件
        Object.keys(JESSIBUCA_EVENTS).forEach((key) => {
            this.player.on(JESSIBUCA_EVENTS[key], (value) => {
                this.emit(key, value)
            })
        })
    }

    /**
     * 是否开启控制台调试打印
     * @param value {Boolean}
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
        this.player.mute(true);
    }

    /**
     *
     */
    cancelMute() {
        this.player.mute(false);
    }

    /**
     *
     * @param value {number}
     */
    setVolume(value) {
        this.player.volume = value;
    }

    /**
     *
     */
    audioResume() {
        this.player.audio && this.player.audio.audioEnabled(true);
    }

    /**
     * 设置超时时长, 单位秒 在连接成功之前和播放中途,如果超过设定时长无数据返回,则回调timeout事件
     * @param value {number}
     */
    setTimeout(time) {
        time = Number(time);
        this.player.updateOption({
            timeout: time,
            loadingTimeout: time,
            heartTimeout: time
        })
    }

    /**
     *
     * @param type {number}: 0,1,2
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
        this.resize();
    }

    /**
     *
     * @returns {Promise<commander.ParseOptionsResult.unknown>}
     */
    pause() {
        return this.player.pause();
    }

    /**
     *
     */
    close() {
        // clear url
        this._opt.url = '';
        return this.player.close();
    }


    /**
     *
     */
    clearView() {
        this.player.video.clearView()
    }

    /**
     *
     * @param url {string}
     * @returns {Promise<unknown>}
     */
    play(url) {
        return new Promise((resolve, reject) => {
            if (!url && !this._opt.url) {
                this.emit(EVENTS.error, EVENTS_ERROR.playError)
                reject();
                return;
            }

            if (url) {
                // url 相等的时候。
                if (this._opt.url) {
                    // 存在相同的 url
                    if (url === this._opt.url) {
                        // 正在播放
                        if (this.player.playing) {
                            resolve();
                        } else {
                            // pause ->  play
                            this.clearView();
                            this.player.play(this._opt.url).then(() => {
                                resolve();
                            }).catch(() => {
                                this.player.pause().then(() => {
                                    reject();
                                })
                            })
                        }
                    } else {
                        // url 发生改变了
                        this.player.pause().then(() => {
                            // 清除 画面
                            this.clearView();
                            return this._play(url);
                        }).catch(() => {
                            reject();
                        })
                    }
                } else {
                    return this._play(url);
                }
            } else {
                //  url 不存在的时候
                //  就是从 play -> pause -> play
                this.player.play(this._opt.url).then(() => {
                    resolve();
                }).catch(() => {
                    this.player.pause().then(() => {
                        reject();
                    })
                })
            }
        })
    }

    /**
     *
     * @param url {string}
     * @returns {Promise<unknown>}
     * @private
     */
    _play(url) {
        return new Promise((resolve, reject) => {
            this._opt.url = url;
            //  新的url
            const isHttp = url.indexOf("http") === 0;
            //
            const protocol = isHttp ? PLAYER_PLAY_PROTOCOL.fetch : PLAYER_PLAY_PROTOCOL.websocket
            //
            const demuxType = (isHttp || url.indexOf(".flv") !== -1 || this._opt.isFlv) ? DEMUX_TYPE.flv : DEMUX_TYPE.m7s;

            this.player.updateOption({
                protocol,
                demuxType
            })

            this.player.once(EVENTS_ERROR.mediaSourceH265NotSupport, () => {
                this.close().then(() => {
                    if (this.player._opt.autoWasm) {
                        this.player.debug.log('Jessibuca', 'auto wasm [mse-> wasm] reset player and play')
                        this._resetPlayer({useMSE: false})
                        this.play(url).then(() => {
                            // resolve();
                            this.player.debug.log('Jessibuca', 'auto wasm [mse-> wasm] reset player and play success')
                        }).catch(() => {
                            // reject();
                            this.player.debug.log('Jessibuca', 'auto wasm [mse-> wasm] reset player and play error')
                        });
                    }
                });
            })

            this.player.once(EVENTS_ERROR.webcodecsH265NotSupport, () => {
                this.close().then(() => {
                    if (this.player._opt.autoWasm) {
                        this.player.debug.log('Jessibuca', 'auto wasm [wcs-> wasm] reset player and play')
                        this._resetPlayer({useWCS: false})
                        this.play(url).then(() => {
                            // resolve();
                            this.player.debug.log('Jessibuca', 'auto wasm [wcs-> wasm] reset player and play success')
                        }).catch(() => {
                            // reject();
                            this.player.debug.log('Jessibuca', 'auto wasm [wcs-> wasm] reset player and play error')
                        });
                    }
                });
            })

            // 解码报错。
            this.player.once(EVENTS_ERROR.wasmDecodeError, () => {
                if (this.player._opt.wasmDecodeErrorReplay) {
                    this.close().then(() => {
                        this.player.debug.log('Jessibuca', 'wasm decode error and reset player and play')
                        this._resetPlayer({useWCS: false})
                        this.play(url).then(() => {
                            // resolve();
                            this.player.debug.log('Jessibuca', 'wasm decode error and reset player and play success')
                        }).catch(() => {
                            // reject();
                            this.player.debug.log('Jessibuca', 'wasm decode error and reset player and play error')
                        });
                    })
                }
            })

            this.player.once(EVENTS.delayTimeout, () => {
                if (this.player._opt.heartTimeoutReplay) {
                    this.play(url).then(() => {
                        // resolve();
                    }).catch(() => {
                        // reject();
                    });
                }
            })

            if (this.hasLoaded()) {
                this.player.play(url).then(() => {
                    resolve();
                }).catch(() => {
                    this.player.pause().then(() => {
                        reject();
                    })
                })
            } else {
                this.player.once(EVENTS.decoderWorkerInit, () => {
                    this.player.play(url).then(() => {
                        resolve();
                    }).catch(() => {
                        this.player.pause().then(() => {
                            reject();
                        })
                    })
                })
            }
        })
    }

    /**
     *
     */
    resize() {
        this.player.resize();
    }

    /**
     *
     * @param time {number} s
     */
    setBufferTime(time) {
        time = Number(time)
        // s -> ms
        this.player.updateOption({
            videoBuffer: time * 1000
        })
        // update worker config
        this.player.decoderWorker && this.player.decoderWorker.updateWorkConfig({
            key: 'videoBuffer',
            value: time * 1000
        })
    }

    /**
     *
     * @param deg {number}
     */
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

    /**
     *
     * @returns {boolean}
     */
    hasLoaded() {
        return this.player.loaded;
    }

    /**
     *
     */
    setKeepScreenOn() {
        this.player.updateOption({
            keepScreenOn: true
        })
    }

    /**
     *
     * @param flag {Boolean}
     */
    setFullscreen(flag) {
        const fullscreen = !!flag;
        if (this.player.fullscreen !== fullscreen) {
            this.player.fullscreen = fullscreen;
        }
    }

    /**
     *
     * @param filename {string}
     * @param format {string}
     * @param quality {number}
     * @param type {string} download,base64,blob
     */
    screenshot(filename, format, quality, type) {
        return this.player.video.screenshot(filename, format, quality, type)
    }

    /**
     *
     * @param fileName {string}
     * @param fileType {string}
     * @returns {Promise<unknown>}
     */
    startRecord(fileName, fileType) {
        return new Promise((resolve, reject) => {
            if (this.player.playing) {
                this.player.startRecord(fileName, fileType)
                resolve();
            } else {
                reject();
            }
        })
    }

    stopRecordAndSave() {
        if (this.player.recording) {
            this.player.stopRecordAndSave();
        }
    }

    /**
     *
     * @returns {Boolean}
     */
    isPlaying() {
        return this.player.playing;
    }

    /**
     * 是否静音状态
     * @returns {Boolean}
     */
    isMute() {
        return this.player.audio ? this.player.audio.isMute : true;
    }

    /**
     * 是否在录制视频
     * @returns {*}
     */
    isRecording() {
        return this.player.recorder.recording;
    }


}


window.Jessibuca = Jessibuca;

export default Jessibuca;
