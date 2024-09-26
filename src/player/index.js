import {DEFAULT_PLAYER_OPTIONS, EVENTS, EVENTS_ERROR, JESSIBUCA_EVENTS, VERSION} from "../constant";
import Debug from "../utils/debug";
import Events from "../utils/events";
import property from './property';
import events from './events';
import {
    fpsStatus,
    initPlayTimes,
    isEmpty, isFalse,
    isFullScreen, isMobile, isPad,
    isNotEmpty,
    now, supportMediaStreamTrack,
    supportMSE,
    supportOffscreenV2,
    supportWCS
} from "../utils";
import Video from "../video";
import Audio from "../audio";
import Stream from "../stream";
import Recorder from "../recorder";
import DecoderWorker from "../worker/index";
import Emitter from "../utils/emitter";
import Demux from "../demux";
import WebcodecsDecoder from "../decoder/webcodecs";
import Control from "../control";
import './style.less'
import observer from "./observer";
import MseDecoder from "../decoder/mediaSource";
import NoSleep from "../utils/noSleep";
import screenfull from "screenfull";

export default class Player extends Emitter {
    constructor(container, options) {
        super()
        this.$container = container;
        this._opt = Object.assign({}, DEFAULT_PLAYER_OPTIONS, options)
        this.debug = new Debug(this);
        this.debug.log('Player', 'init');

        // disable offscreen
        this._opt.forceNoOffscreen = true;

        if (isMobile() || isPad()) {
            this.debug.log('Player', 'isMobile and set _opt.controlAutoHide false');
            this._opt.controlAutoHide = false;
        }

        if (this._opt.autoUseSystemFullScreen) {
            if (screenfull.isEnabled &&
                this._opt.useWebFullScreen) {
                this.debug.log('Player', 'screenfull.isEnabled is true and _opt.useWebFullScreen is true , set _opt.useWebFullScreen false');
                this._opt.useWebFullScreen = false;
            }

            if (isFalse(screenfull.isEnabled) &&
                isFalse(this._opt.useWebFullScreen)) {
                this.debug.log('Player', 'screenfull.isEnabled is false and _opt.useWebFullScreen is false , set _opt.useWebFullScreen true');
                this._opt.useWebFullScreen = true;
            }
        }
        //
        if (this._opt.useWCS) {
            this._opt.useWCS = supportWCS();
        }

        //
        if (this._opt.useMSE) {
            this._opt.useMSE = supportMSE();
        }

        //
        if (this._opt.wcsUseVideoRender) {
            this._opt.wcsUseVideoRender = supportMediaStreamTrack()
        }

        // 如果使用mse则强制不允许 webcodecs
        if (this._opt.useMSE) {
            if (this._opt.useWCS) {
                this.debug.log('Player', 'useWCS set true->false')
            }

            if (!this._opt.forceNoOffscreen) {
                this.debug.log('Player', 'forceNoOffscreen set false->true')
            }

            this._opt.useWCS = false;
            this._opt.forceNoOffscreen = true;
        }

        if (!this._opt.forceNoOffscreen) {
            if (!supportOffscreenV2()) {
                this._opt.forceNoOffscreen = true;
                this._opt.useOffscreen = false;
            } else {
                this._opt.useOffscreen = true;
            }
        }

        if (!this._opt.hasAudio) {
            this._opt.operateBtns.audio = false
        }

        this._opt.hasControl = this._hasControl();
        //
        this._loading = false;
        this._playing = false;
        this._hasLoaded = false;
        this._destroyed = false;
        this._closed = false;

        //
        this._checkHeartTimeout = null;
        this._checkLoadingTimeout = null;
        this._checkStatsInterval = null;

        //
        this._startBpsTime = null;
        this._isPlayingBeforePageHidden = false;
        this._stats = {
            buf: 0, // 当前缓冲区时长，单位毫秒,
            fps: 0, // 当前视频帧率
            abps: 0, // 当前音频码率，单位bit
            vbps: 0, // 当前视频码率，单位bit
            ts: 0 // 当前视频帧pts，单位毫秒
        }

        // 各个步骤的时间统计
        this._times = initPlayTimes();

        //
        this._videoTimestamp = 0;
        this._audioTimestamp = 0;

        property(this);

        this.events = new Events(this);
        this.video = new Video(this);

        if (this._opt.hasAudio) {
            this.audio = new Audio(this);
        }
        this.recorder = new Recorder(this);

        if (!this._onlyMseOrWcsVideo()) {
            this.decoderWorker = new DecoderWorker(this);
        } else {
            this.loaded = true;
        }

        this.stream = null;
        this.demux = null;
        this._lastVolume = null;

        if (this._opt.useWCS) {
            this.webcodecsDecoder = new WebcodecsDecoder(this)
            this.loaded = true
        }

        if (this._opt.useMSE) {
            this.mseDecoder = new MseDecoder(this);
            this.loaded = true
        }

        //
        this.control = new Control(this);

        if (isMobile()) {
            this.keepScreenOn = new NoSleep(this);
        }

        events(this);
        observer(this);
        this.debug.log('Player', 'init and version is', VERSION);

        if (this._opt.useWCS) {
            this.debug.log('Player', 'use WCS')
        }

        if (this._opt.useMSE) {
            this.debug.log('Player', 'use MSE')
        }

        if (this._opt.useOffscreen) {
            this.debug.log('Player', 'use offscreen')
        }

        try {
            this.debug.log('Player options', JSON.stringify(this._opt));
        } catch (e) {
            // ignore
        }
    }


    async destroy() {
        this._destroyed = true;
        this._loading = false;
        this._playing = false;
        this._hasLoaded = false;
        this._lastVolume = null;
        this._times = initPlayTimes();

        if (this.decoderWorker) {
            await this.decoderWorker.destroy();
            this.decoderWorker = null;
        }
        if (this.video) {
            this.video.destroy();
            this.video = null;
        }

        if (this.audio) {
            this.audio.destroy();
            this.audio = null;
        }

        if (this.stream) {
            await this.stream.destroy();
            this.stream = null;
        }

        if (this.recorder) {
            this.recorder.destroy();
            this.recorder = null;
        }

        if (this.control) {
            this.control.destroy();
            this.control = null;
        }

        if (this.webcodecsDecoder) {
            this.webcodecsDecoder.destroy();
            this.webcodecsDecoder = null;
        }

        if (this.mseDecoder) {
            this.mseDecoder.destroy();
            this.mseDecoder = null;
        }

        if (this.demux) {
            this.demux.destroy();
            this.demux = null;
        }


        if (this.events) {
            this.events.destroy();
            this.events = null;
        }

        this.clearCheckHeartTimeout();
        this.clearCheckLoadingTimeout();
        this.clearStatsInterval();
        //
        this.releaseWakeLock();
        this.keepScreenOn = null;
        // reset stats
        this.resetStats();
        this._audioTimestamp = 0;
        this._videoTimestamp = 0;

        // 其他没法解耦的，通过 destroy 方式
        this.emit('destroy');
        // 接触所有绑定事件
        this.off();

        this.debug.log('play', 'destroy end');
    }

    set fullscreen(value) {
        if (isMobile() && this._opt.useWebFullScreen) {
            this.emit(EVENTS.webFullscreen, value);
            setTimeout(() => {
                this.updateOption({
                    rotate: value ? 270 : 0,
                })
                this.resize();
            }, 10)
        } else {
            this.emit(EVENTS.fullscreen, value);
        }
    }

    get fullscreen() {
        return isFullScreen() || this.webFullscreen;
    }

    set webFullscreen(value) {
        this.emit(EVENTS.webFullscreen, value);
    }

    get webFullscreen() {
        return this.$container.classList.contains('jessibuca-fullscreen-web')
    }

    set loaded(value) {
        this._hasLoaded = value;
    }

    get loaded() {
        return this._hasLoaded;
    }

    //
    set playing(value) {

        if (value) {
            // 将loading 设置为 false
            this.loading = false;
        }

        if (this.playing !== value) {
            this._playing = value;
            this.emit(EVENTS.playing, value);
            this.emit(EVENTS.volumechange, this.volume);

            if (value) {
                this.emit(EVENTS.play);
            } else {
                this.emit(EVENTS.pause);
            }
        }
    }

    get playing() {
        return this._playing;
    }

    get volume() {
        return (this.audio && this.audio.volume) || 0;
    }

    set volume(value) {
        if (value !== this.volume) {
            this.audio && this.audio.setVolume(value);
            this._lastVolume = value;
        }
    }

    get lastVolume() {
        return this._lastVolume
    }

    set loading(value) {
        if (this.loading !== value) {
            this._loading = value;
            this.emit(EVENTS.loading, this._loading);
        }
    }

    get loading() {
        return this._loading;
    }

    set recording(value) {
        if (value) {
            if (this.playing) {
                this.recorder && this.recorder.startRecord();
            }
        } else {
            this.recorder && this.recorder.stopRecordAndSave();
        }
    }

    get recording() {
        return this.recorder ? this.recorder.recording : false;
    }

    set audioTimestamp(value) {
        if (value === null) {
            return;
        }
        this._audioTimestamp = value;
    }

    //
    get audioTimestamp() {
        return this._audioTimestamp;
    }

    //
    set videoTimestamp(value) {
        if (value === null) {
            return;
        }
        this._videoTimestamp = value;
        // just for wasm
        if (!this._opt.useWCS && !this._opt.useMSE) {
            if (this.audioTimestamp && this.videoTimestamp) {
                this.audio && this.audio.emit(EVENTS.videoSyncAudio, {
                    audioTimestamp: this.audioTimestamp,
                    videoTimestamp: this.videoTimestamp,
                    diff: this.audioTimestamp - this.videoTimestamp
                })
            }
        }
    }

    //
    get videoTimestamp() {
        return this._videoTimestamp;
    }

    get isDebug() {
        return this._opt.debug === true;
    }


    /**
     *
     * @param options
     */
    updateOption(options) {
        this._opt = Object.assign({}, this._opt, options)
    }

    /**
     *
     * @returns {Promise<unknown>}
     */
    init() {
        return new Promise((resolve, reject) => {
            if (!this.stream) {
                this.stream = new Stream(this);
            }

            if (!this.audio) {
                if (this._opt.hasAudio) {
                    this.audio = new Audio(this);
                }
            }
            if (!this.demux) {
                this.demux = new Demux(this);
            }

            if (this._opt.useWCS) {
                if (!this.webcodecsDecoder) {
                    this.webcodecsDecoder = new WebcodecsDecoder(this)
                }
            }

            if (this._opt.useMSE) {
                if (!this.mseDecoder) {
                    this.mseDecoder = new MseDecoder(this);
                }
            }

            if (!this.decoderWorker && !this._onlyMseOrWcsVideo()) {
                this.decoderWorker = new DecoderWorker(this);
                this.debug.log('Player', 'waiting decoderWorker init');
                this.once(EVENTS.decoderWorkerInit, () => {
                    if (this.isDestroyedOrClosed()) {
                        reject('init() failed and player is destroyed or closed');
                    } else {
                        this.debug.log('Player', 'decoderWorker init success');
                        this.loaded = true;
                        resolve()
                    }
                })
            } else {
                resolve()
            }

        })
    }


    /**
     *
     * @param url
     * @returns {Promise<unknown>}
     */
    play(url, options) {
        return new Promise((resolve, reject) => {
            if (!url && !this._opt.url) {
                return reject();
            }

            this.loading = true;
            this.playing = false;
            this._times.playInitStart = now();
            if (!url) {
                url = this._opt.url;
            }
            this._opt.url = url;

            this.clearCheckHeartTimeout();

            this.init().then(() => {
                this._times.playStart = now();
                //
                if (this._opt.isNotMute) {
                    this.mute(false);
                }


                if (this.webcodecsDecoder) {
                    this.webcodecsDecoder.once(EVENTS_ERROR.webcodecsH265NotSupport, () => {
                        this.emit(EVENTS_ERROR.webcodecsH265NotSupport)
                        if (!this._opt.autoWasm) {
                            this.emit(EVENTS.error, EVENTS_ERROR.webcodecsH265NotSupport);
                        }
                    })
                }

                if (this.mseDecoder) {
                    this.mseDecoder.once(EVENTS_ERROR.mediaSourceH265NotSupport, () => {
                        this.emit(EVENTS_ERROR.mediaSourceH265NotSupport)
                        if (!this._opt.autoWasm) {
                            this.emit(EVENTS.error, EVENTS_ERROR.mediaSourceH265NotSupport);
                        }
                    })

                    this.mseDecoder.once(EVENTS_ERROR.mediaSourceFull, () => {
                        this.emitError(EVENTS_ERROR.mediaSourceFull)
                    })

                    this.mseDecoder.once(EVENTS_ERROR.mediaSourceAppendBufferError, () => {
                        this.emitError(EVENTS_ERROR.mediaSourceAppendBufferError)
                    })

                    this.mseDecoder.once(EVENTS_ERROR.mediaSourceBufferListLarge, () => {
                        this.emitError(EVENTS_ERROR.mediaSourceBufferListLarge);
                    })

                    this.mseDecoder.once(EVENTS_ERROR.mediaSourceAppendBufferEndTimeout, () => {
                        this.emitError(EVENTS_ERROR.mediaSourceAppendBufferEndTimeout);
                    })
                }

                this.enableWakeLock();

                this.stream.fetchStream(url, options);

                //
                this.checkLoadingTimeout();
                // fetch error
                this.stream.once(EVENTS_ERROR.fetchError, (error) => {
                    // reject(error)
                    this.emitError(EVENTS_ERROR.fetchError, error);
                })

                // ws
                this.stream.once(EVENTS_ERROR.websocketError, (error) => {
                    // reject(error)
                    this.emitError(EVENTS_ERROR.websocketError, error);
                })

                // stream end
                this.stream.once(EVENTS.streamEnd, (msg) => {
                    // reject();
                    this.emitError(EVENTS.streamEnd, msg);
                })

                // success
                this.stream.once(EVENTS.streamSuccess, () => {
                    resolve();
                    this._times.streamResponse = now();
                    //
                    this.video.play();
                    this.checkStatsInterval();
                })

            }).catch((e) => {
                reject(e)
            })
        })
    }

    /**
     *
     */
    close() {
        return new Promise((resolve, reject) => {
            this._close().then(() => {
                this.video && this.video.clearView();
                resolve()
            })
        })
    }

    resumeAudioAfterPause() {
        if (this.lastVolume) {
            this.volume = this.lastVolume;
        }
    }

    _close() {
        return new Promise((resolve, reject) => {
            this._closed = true;
            //
            if (this.stream) {
                this.stream.destroy();
                this.stream = null;
            }

            if (this.demux) {
                this.demux.destroy();
                this.demux = null;
            }

            //
            if (this.decoderWorker) {
                this.decoderWorker.destroy();
                this.decoderWorker = null;
            }

            if (this.webcodecsDecoder) {
                this.webcodecsDecoder.destroy();
                this.webcodecsDecoder = null;
            }

            if (this.mseDecoder) {
                this.mseDecoder.destroy();
                this.mseDecoder = null;
            }

            if (this.audio) {
                this.audio.destroy();
                this.audio = null;
            }
            this.clearCheckHeartTimeout();
            this.clearCheckLoadingTimeout();
            this.clearStatsInterval();
            this.playing = false;
            this.loading = false;
            this.recording = false;

            if (this.video) {
                this.video.resetInit();
                this.video.pause(true);
            }
            // release lock
            this.releaseWakeLock();
            // reset stats
            this.resetStats();
            //
            this._audioTimestamp = 0;
            this._videoTimestamp = 0;
            //
            this._times = initPlayTimes();
            //
            setTimeout(() => {
                resolve()
            }, 0)
        })
    }

    /**
     *
     * @param flag {boolean} 是否清除画面
     * @returns {Promise<unknown>}
     */
    pause(flag = false) {
        if (flag) {
            return this.close();
        } else {
            return this._close();
        }
    }

    /**
     *
     * @param flag
     */
    mute(flag) {
        if (this.audio) {
            const prev = this.audio.getLastVolume();
            this.audio.mute(flag)
            if (flag) {
                this._lastVolume = 0;
            } else {
                this._lastVolume = prev || 0.5;
            }
        }
    }

    /**
     *
     */
    resize() {
        this.video.resize();
    }

    /**
     *
     * @param fileName
     * @param fileType
     */
    startRecord(fileName, fileType) {
        if (this.recording) {
            return;
        }

        this.recorder.setFileName(fileName, fileType);
        this.recording = true;
    }

    /**
     *
     */
    stopRecordAndSave() {
        if (this.recording) {
            this.recording = false;
        }
    }

    _hasControl() {
        let result = false;

        let hasBtnShow = false;
        Object.keys(this._opt.operateBtns).forEach((key) => {
            if (this._opt.operateBtns[key]) {
                hasBtnShow = true;
            }
        });

        if (this._opt.showBandwidth || this._opt.text || hasBtnShow) {
            result = true;
        }

        return result;
    }

    _onlyMseOrWcsVideo() {
        return this._opt.hasAudio === false && (this._opt.useMSE || (this._opt.useWCS && !this._opt.useOffscreen))
    }

    checkHeart() {
        this.clearCheckHeartTimeout();
        this.checkHeartTimeout();
    }

    // 心跳检查，如果渲染间隔暂停了多少时间之后，就会抛出异常
    checkHeartTimeout() {
        this._checkHeartTimeout = setTimeout(() => {
            if (this.playing) {
                // check again
                if (this._stats.fps !== 0) {
                    return;
                }
                if (this.isDestroyedOrClosed()) {
                    return;
                }

                this.pause().then(() => {
                    this.emit(EVENTS.timeout, EVENTS.delayTimeout);
                    this.emit(EVENTS.delayTimeout);
                });
            }

        }, this._opt.heartTimeout * 1000)
    }

    checkStatsInterval() {
        this._checkStatsInterval = setInterval(() => {
            this.updateStats();
        }, 1000)
    }

    //
    clearCheckHeartTimeout() {
        if (this._checkHeartTimeout) {
            clearTimeout(this._checkHeartTimeout);
            this._checkHeartTimeout = null;
        }
    }

    // loading 等待时间
    checkLoadingTimeout() {
        this._checkLoadingTimeout = setTimeout(() => {
            // check again
            if (this.playing) {
                return;
            }
            if (this.isDestroyedOrClosed()) {
                return;
            }
            this.pause().then(() => {
                this.emit(EVENTS.timeout, EVENTS.loadingTimeout);
                this.emit(EVENTS.loadingTimeout);
            });
        }, this._opt.loadingTimeout * 1000)
    }

    clearCheckLoadingTimeout() {
        if (this._checkLoadingTimeout) {
            clearTimeout(this._checkLoadingTimeout);
            this._checkLoadingTimeout = null;
        }
    }

    clearStatsInterval() {
        if (this._checkStatsInterval) {
            clearInterval(this._checkStatsInterval);
            this._checkStatsInterval = null;
        }
    }

    handleRender() {
        if (this.isDestroyedOrClosed()) {
            return;
        }

        if (this.loading) {
            this.emit(EVENTS.start);
            this.loading = false;
            this.clearCheckLoadingTimeout();
        }
        if (!this.playing) {
            this.playing = true;
        }
        this.checkHeart();
    }


    //
    updateStats(options = {}) {
        if (this.isDestroyedOrClosed()) {
            return;
        }

        if (!this._startBpsTime) {
            this._startBpsTime = now();
        }

        if (isNotEmpty(options.ts)) {
            this._stats.ts = options.ts;
        }

        if (isNotEmpty(options.buf)) {
            this._stats.buf = options.buf;
        }

        if (options.fps) {
            this._stats.fps += 1;
        }
        if (options.abps) {
            this._stats.abps += options.abps;
        }
        if (options.vbps) {
            this._stats.vbps += options.vbps;
        }

        const _nowTime = now();
        const timestamp = _nowTime - this._startBpsTime;

        if (timestamp < 1 * 1000) {
            return;
        }

        this.emit(EVENTS.stats, this._stats);
        this.emit(EVENTS.performance, fpsStatus(this._stats.fps));
        this._stats.fps = 0;
        this._stats.abps = 0;
        this._stats.vbps = 0;
        this._startBpsTime = _nowTime;
    }

    resetStats() {
        this._startBpsTime = null;
        this._stats = {
            buf: 0, //ms
            fps: 0,
            abps: 0,
            vbps: 0,
            ts: 0
        }
    }

    enableWakeLock() {
        if (this._opt.keepScreenOn) {
            this.keepScreenOn && this.keepScreenOn.enable();
        }
    }

    releaseWakeLock() {
        if (this._opt.keepScreenOn) {
            this.keepScreenOn && this.keepScreenOn.disable();
        }
    }

    handlePlayToRenderTimes() {
        if (this.isDestroyedOrClosed()) {
            return;
        }

        const _times = this._times;
        _times.playTimestamp = _times.playStart - _times.playInitStart;
        _times.streamTimestamp = _times.streamStart - _times.playStart;
        _times.streamResponseTimestamp = _times.streamResponse - _times.streamStart;
        _times.demuxTimestamp = _times.demuxStart - _times.streamResponse;
        _times.decodeTimestamp = _times.decodeStart - _times.demuxStart;
        _times.videoTimestamp = _times.videoStart - _times.decodeStart;
        _times.allTimestamp = _times.videoStart - _times.playInitStart;
        this.emit(EVENTS.playToRenderTimes, _times);
    }

    getOption() {
        return this._opt;
    }

    emitError(errorType, message = '') {
        this.emit(EVENTS.error, errorType, message);
        this.emit(errorType, message);
    }

    isControlBarShow() {
        const hasControl = this._opt.hasControl;
        const controlAutoHide = this._opt.controlAutoHide;

        let result = hasControl && !controlAutoHide;

        if (result) {
            if (this.control) {
                result = this.control.getBarIsShow();
            }
        }

        return result;
    }

    getControlBarShow() {
        let result = false;
        if (this.control) {
            result = this.control.getBarIsShow();
        }
        return result;
    }

    toggleControlBar(isShow) {
        if (this.control) {
            this.control.toggleBar(isShow);
            this.resize();
        }
    }

    isDestroyed() {
        return this._destroyed;
    }

    isClosed() {
        return this._closed;
    }

    isDestroyedOrClosed() {
        return this.isDestroyed() || this.isClosed();
    }
}
