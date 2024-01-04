import Player from './player';
import Events from "./utils/events";
import {DEMUX_TYPE, EVENTS, EVENTS_ERROR, JESSIBUCA_EVENTS, PLAYER_PLAY_PROTOCOL, SCALE_MODE_TYPE} from "./constant";
import {isEmpty, isMobile, isNotEmpty, supportWCS, uuid16} from "./utils";
import Emitter from "./utils/emitter";
import Debug from "./utils/debug";


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
        // check container node name
        if ($container.nodeName === 'CANVAS' || $container.nodeName === 'VIDEO') {
            throw new Error(`Jessibuca container type can not be ${$container.nodeName} type`);
            return;
        }

        if (_opt.videoBuffer >= _opt.heartTimeout) {
            throw new Error(`Jessibuca videoBuffer ${_opt.videoBuffer}s must be less than heartTimeout ${_opt.heartTimeout}s`);
            return;
        }


        $container.classList.add('jessibuca-container');

        delete _opt.container;

        // 禁用离屏渲染
        _opt.forceNoOffscreen = true;

        // 移动端不支持自动关闭控制栏
        if (isMobile()) {
            _opt.controlAutoHide = false;
        }

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
        this._loadingTimeoutReplayTimes = 0;
        this._heartTimeoutReplayTimes = 0;
        this.events = new Events(this);
        this.debug = new Debug(this);
        this._initPlayer($container, _opt);
    }

    /**
     *
     */
    destroy() {
        if (this.events) {
            this.events.destroy();
            this.events = null;
        }

        if (this.player) {
            this.player.destroy();
            this.player = null;
        }
        this.$container = null;
        this._opt = null;
        this._loadingTimeoutReplayTimes = 0;
        this._heartTimeoutReplayTimes = 0;
        this.off();
    }

    _initPlayer($container, options) {
        this.player = new Player($container, options);
        this.debug.log('jessibuca', '_initPlayer', this.player.getOption())
        this._bindEvents();
    }

    _resetPlayer(options = {}) {
        this.player.destroy();
        this.player = null;
        this._opt = Object.assign(this._opt, options);
        this._opt.url = '';// reset url
        this._initPlayer(this.$container, this._opt);
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
            debug: !!value
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
        return new Promise((resolve, reject) => {
            if (this.player) {
                this.player.pause().then(() => {
                    resolve();
                }).catch((e) => {
                    reject(e)
                });
            } else {
                reject('player is null')
            }
        })
    }

    /**
     *
     */
    close() {
        // clear url
        this._opt.url = '';
        this._opt.playOptions = {};
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
     * @param options {object}
     * @returns {Promise<unknown>}
     */
    play(url, options = {}) {
        return new Promise((resolve, reject) => {
            if (!url && !this._opt.url) {
                this.emit(EVENTS.error, EVENTS_ERROR.playError)
                reject('play url is empty')
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
                            this.player.play(this._opt.url, this._opt.playOptions).then(() => {
                                resolve();
                                // 恢复下之前的音量
                                this.player.resumeAudioAfterPause();
                            }).catch((e) => {
                                this.debug.warn('jessibuca', 'pause ->  play and play error', e)
                                this.player.pause().then(() => {
                                    reject(e);
                                })
                            })
                        }
                    } else {
                        // url 发生改变了
                        this.player.pause().then(() => {
                            // 清除 画面
                            this.clearView();
                            this._play(url, options).then(() => {
                                resolve()
                            }).catch((e) => {
                                this.debug.warn('jessibuca', 'this._play error', e)
                                reject(e)
                            });
                        }).catch((e) => {
                            this.debug.warn('jessibuca', 'this._opt.url is null and pause error', e)
                            reject(e);
                        })
                    }
                } else {
                    this._play(url, options).then(() => {
                        resolve()
                    }).catch((e) => {
                        this.debug.warn('jessibuca', 'this._play error', e)
                        reject(e)
                    });
                }
            } else {
                //  url 不存在的时候
                //  就是从 play -> pause -> play
                this.player.play(this._opt.url, this._opt.playOptions).then(() => {
                    resolve();
                    // 恢复下之前的音量
                    this.player.resumeAudioAfterPause();
                }).catch((e) => {
                    this.debug.warn('jessibuca', 'url is null and play error', e)
                    this.player.pause().then(() => {
                        reject(e);
                    })
                })
            }
        })
    }

    /**
     *
     * @param url {string}
     * @param options {object}
     * @returns {Promise<unknown>}
     * @private
     */
    _play(url, options = {}) {
        return new Promise((resolve, reject) => {
            this._opt.url = url;
            this._opt.playOptions = options;
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

            this.player.once(EVENTS_ERROR.webglAlignmentError, () => {
                this.pause().then(() => {
                    this.debug.log('Jessibuca', 'webglAlignmentError')
                    this._resetPlayer({openWebglAlignment: true})
                    this.play(url, options).then(() => {
                        // resolve();
                        this.debug.log('Jessibuca', 'webglAlignmentError and play success')
                    }).catch(() => {
                        // reject();
                        this.debug.log('Jessibuca', 'webglAlignmentError and play error')
                    });
                });
            })


            this.player.once(EVENTS_ERROR.mediaSourceH265NotSupport, () => {
                this.pause().then(() => {
                    if (this.player._opt.autoWasm) {
                        this.debug.log('Jessibuca', 'auto wasm [mse-> wasm] reset player and play')
                        this._resetPlayer({useMSE: false})
                        this.play(url, options).then(() => {
                            // resolve();
                            this.debug.log('Jessibuca', 'auto wasm [mse-> wasm] reset player and play success')
                        }).catch(() => {
                            // reject();
                            this.debug.log('Jessibuca', 'auto wasm [mse-> wasm] reset player and play error')
                        });
                    }
                });
            })

            //  media source full error
            this.player.once(EVENTS_ERROR.mediaSourceFull, () => {
                this.pause().then(() => {
                    this.debug.log('Jessibuca', 'media source full');
                    this._resetPlayer()
                    this.play(url, options).then(() => {
                        // resolve();
                        this.debug.log('Jessibuca', 'media source full and reset player and play success')
                    }).catch(() => {
                        // reject();
                        this.debug.warn('Jessibuca', 'media source full and reset player and play error')
                    });
                })
            })
            // media source append buffer error
            this.player.once(EVENTS_ERROR.mediaSourceAppendBufferError, () => {
                this.pause().then(() => {
                    this.debug.log('Jessibuca', 'media source append buffer error');
                    this._resetPlayer()
                    this.play(url, options).then(() => {
                        // resolve();
                        this.debug.log('Jessibuca', 'media source append buffer error and reset player and play success')
                    }).catch(() => {
                        // reject();
                        this.debug.warn('Jessibuca', 'media source append buffer error and reset player and play error')
                    });
                })
            })

            this.player.once(EVENTS_ERROR.mediaSourceBufferListLarge, () => {
                this.pause().then(() => {
                    this.debug.log('Jessibuca', 'media source buffer list large');
                    this._resetPlayer()
                    this.play(url, options).then(() => {
                        // resolve();
                        this.debug.log('Jessibuca', 'media source buffer list large and reset player and play success')
                    }).catch(() => {
                        // reject();
                        this.debug.warn('Jessibuca', 'media source buffer list large and reset player and play error')
                    });
                })
            })

            this.player.once(EVENTS_ERROR.mediaSourceAppendBufferEndTimeout, () => {
                this.pause().then(() => {
                    this.debug.log('Jessibuca', 'media source append buffer end timeout');
                    this._resetPlayer()
                    this.play(url, options).then(() => {
                        // resolve();
                        this.debug.log('Jessibuca', 'media source append buffer end timeout and reset player and play success')
                    }).catch(() => {
                        // reject();
                        this.debug.warn('Jessibuca', 'media source append buffer end timeout and reset player and play error')
                    });
                })
            })

            this.player.once(EVENTS_ERROR.mseSourceBufferError, () => {
                this.pause().then(() => {
                    if (this.player._opt.autoWasm) {
                        this.debug.log('Jessibuca', 'auto wasm [mse-> wasm] reset player and play')
                        this._resetPlayer({useMSE: false})
                        this.play(url, options).then(() => {
                            // resolve();
                            this.debug.log('Jessibuca', 'auto wasm [mse-> wasm] reset player and play success')
                        }).catch(() => {
                            // reject();
                            this.debug.warn('Jessibuca', 'auto wasm [mse-> wasm] reset player and play error')
                        });
                    }
                })
            })

            //
            this.player.once(EVENTS_ERROR.webcodecsH265NotSupport, () => {
                this.pause().then(() => {
                    if (this.player._opt.autoWasm) {
                        this.debug.log('Jessibuca', 'auto wasm [wcs-> wasm] reset player and play')
                        this._resetPlayer({useWCS: false})
                        this.play(url, options).then(() => {
                            // resolve();
                            this.debug.log('Jessibuca', 'auto wasm [wcs-> wasm] reset player and play success')
                        }).catch(() => {
                            // reject();
                            this.debug.warn('Jessibuca', 'auto wasm [wcs-> wasm] reset player and play error')
                        });
                    }
                });
            })
            // webcodecs
            this.player.once(EVENTS_ERROR.webcodecsWidthOrHeightChange, () => {
                this.pause().then(() => {
                    this.debug.log('Jessibuca', 'webcodecs Width Or Height Change reset player and play')
                    this._resetPlayer({useWCS: true})
                    this.play(url, options).then(() => {
                        // resolve();
                        this.debug.log('Jessibuca', 'webcodecs Width Or Height Change reset player and play success')
                    }).catch(() => {
                        // reject();
                        this.debug.warn('Jessibuca', 'webcodecs Width Or Height Change reset player and play error')
                    });
                });
            })
            // webcodecs
            this.player.once(EVENTS_ERROR.webcodecsDecodeError, () => {
                this.pause().then(() => {
                    if (this.player._opt.autoWasm) {
                        this.debug.log('Jessibuca', 'webcodecs decode error reset player and play')
                        this._resetPlayer({useWCS: false})
                        this.play(url, options).then(() => {
                            // resolve();
                            this.debug.log('Jessibuca', 'webcodecs decode error  reset player and play success')
                        }).catch(() => {
                            // reject();
                            this.debug.warn('Jessibuca', 'webcodecs decode error reset player and play error')
                        });
                    }
                });
            })
            // wasm。
            this.player.once(EVENTS_ERROR.wasmDecodeError, () => {
                if (this.player._opt.wasmDecodeErrorReplay) {
                    this.pause().then(() => {
                        this.debug.log('Jessibuca', 'wasm decode error and reset player and play')
                        this._resetPlayer({useWCS: false})
                        this.play(url, options).then(() => {
                            // resolve();
                            this.debug.log('Jessibuca', 'wasm decode error and reset player and play success')
                        }).catch(() => {
                            // reject();
                            this.debug.warn('Jessibuca', 'wasm decode error and reset player and play error')
                        });
                    })
                }
            })


            // 监听 delay timeout
            this.player.on(EVENTS.delayTimeout, () => {
                if (this.player._opt.heartTimeoutReplay && (this._heartTimeoutReplayTimes < this.player._opt.heartTimeoutReplayTimes || this.player._opt.heartTimeoutReplayTimes === -1)) {
                    this.debug.log('Jessibuca', `delay timeout replay time is ${this._heartTimeoutReplayTimes}`)
                    this._heartTimeoutReplayTimes += 1;
                    this.play(url, options).then(() => {
                        // resolve();
                        this._heartTimeoutReplayTimes = 0;
                    }).catch(() => {
                        // reject();
                    });
                }
            })

            // 监听 loading timeout
            this.player.on(EVENTS.loadingTimeout, () => {
                if (this.player._opt.loadingTimeoutReplay && (this._loadingTimeoutReplayTimes < this.player._opt.loadingTimeoutReplayTimes || this.player._opt.loadingTimeoutReplayTimes === -1)) {
                    this.debug.log('Jessibuca', `loading timeout replay time is ${this._loadingTimeoutReplayTimes}`)
                    this._loadingTimeoutReplayTimes += 1;
                    this.play(url, options).then(() => {
                        // resolve();
                        this._loadingTimeoutReplayTimes = 0;
                    }).catch(() => {
                        // reject();
                    });
                }
            })


            if (this.hasLoaded()) {
                this.player.play(url, options).then(() => {
                    resolve();
                }).catch((e) => {
                    this.debug.warn('Jessibuca', 'hasLoaded and play error', e)
                    this.player && this.player.pause().then(() => {
                        reject(e);
                    })
                })
            } else {
                this.player.once(EVENTS.decoderWorkerInit, () => {
                    this.player.play(url, options).then(() => {
                        resolve();
                    }).catch((e) => {
                        this.debug.warn('Jessibuca', 'decoderWorkerInit and play error', e)
                        this.player && this.player.pause().then(() => {
                            reject(e);
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
        const list = [0, 90, 180, 270];
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

        if (!this.player.video) {
            return ''
        }

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
        return this.player ? this.player.playing : false;
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

