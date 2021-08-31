import initVideo from "./video/canvas";
import initAudio from "./audio/audioContext";
import initDashboard from './dashboard/dashboard';
import initDecodeWorker from './worker/decodeWorker';
import initCore from './core/index';
import {DEFAULT_OPTIONS, EVEMTS, POST_MESSAGE} from "./constant";
import {$domToggle, $hideBtns, $initBtns, checkFull} from "./utils";

class Jessibuca {
    constructor(options) {
        this._opt = Object.assign(DEFAULT_OPTIONS, options);
        this.$container = options.container;
        if (typeof options.container === 'string') {
            this.$container = document.querySelector(options.container);
        }
        if (!this.$container) {
            throw new Error('Jessibuca need container option');
            return;
        }
        delete this._opt.container;

        this._opt.debug && console.log('options', this._opt);
        // core
        initCore(this);
        // video
        initVideo(this);
        // audio
        initAudio(this);
        // decode worker
        initDecodeWorker(this);
        // dashboard
        initDashboard(this);
    }

    set fullscreen(value) {
        if (value) {
            if (!checkFull()) {
                this.$container.requestFullscreen();
            }
            $domToggle(this.$doms.minScreenDom, true);
            $domToggle(this.$doms.fullscreenDom, false);
        } else {
            if (checkFull()) {
                document.exitFullscreen();
            }
            $domToggle(this.$doms.minScreenDom, false);
            $domToggle(this.$doms.fullscreenDom, true);
        }

        if (this._fullscreen !== value) {
            this.onFullscreen(value);
            this._trigger(EVEMTS.fullscreen, value);
        }


        this._fullscreen = value;
    }

    get fullscreen() {
        return this._fullscreen;
    }

    set playing(value) {
        if (value) {
            $domToggle(this.$doms.playBigDom, false);
            $domToggle(this.$doms.playDom, false);
            $domToggle(this.$doms.pauseDom, true);

            $domToggle(this.$doms.screenshotsDom, true);
            $domToggle(this.$doms.recordDom, true);
            if (this._quieting) {
                $domToggle(this.$doms.quietAudioDom, true);
                $domToggle(this.$doms.playAudioDom, false);
            } else {
                $domToggle(this.$doms.quietAudioDom, false);
                $domToggle(this.$doms.playAudioDom, true);
            }
        } else {
            this.$doms.speedDom && (this.$doms.speedDom.innerText = '');
            if (this._playUrl) {
                $domToggle(this.$doms.playDom, true);
                $domToggle(this.$doms.playBigDom, true);
                $domToggle(this.$doms.pauseDom, false);
            }

            // 在停止状态下录像，截屏，音量是非激活，只有播放,最大化时可点击
            $domToggle(this.$doms.recordDom, false);
            $domToggle(this.$doms.recordingDom, false);
            $domToggle(this.$doms.screenshotsDom, false);
            $domToggle(this.$doms.quietAudioDom, false);
            $domToggle(this.$doms.playAudioDom, false);
        }

        if (this._playing !== value) {
            if (value) {
                this.onPlay();
                this._trigger(EVEMTS.play);
            } else {
                this.onPause();
                this._trigger(EVEMTS.pause);
            }
        }
        this._playing = value;
    }

    get playing() {
        return this._playing;
    }

    set quieting(value) {
        if (value) {
            $domToggle(this.$doms.quietAudioDom, true);
            $domToggle(this.$doms.playAudioDom, false);
        } else {
            $domToggle(this.$doms.quietAudioDom, false);
            $domToggle(this.$doms.playAudioDom, true);
        }
        if (this._quieting !== value) {
            this.onMute(value);
            this._trigger(EVEMTS.mute, value);
        }
        this._quieting = value;
    }

    get quieting() {
        return this._quieting;
    }

    set loading(value) {
        if (value) {
            $hideBtns(this.$doms)
            $domToggle(this.$doms.fullscreenDom, true);
            $domToggle(this.$doms.pauseDom, true);
            $domToggle(this.$doms.loadingDom, true);
        } else {
            $initBtns(this.$doms);
        }
        this._loading = value;
    }

    get loading() {
        return this._loading;
    }

    set recording(value) {
        if (value) {
            $domToggle(this.$doms.recordDom, false);
            $domToggle(this.$doms.recordingDom, true);
        } else {
            $domToggle(this.$doms.recordDom, true);
            $domToggle(this.$doms.recordingDom, false);

        }
        if (this._recording !== value) {
            this.onRecord(value);
            this._trigger(EVEMTS.record, value);
            this._recording = value;
        }
    }

    get recording() {
        return this._recording;
    }

    /**
     * 是否正在播放
     * @returns {*}
     */
    isPlaying() {
        return this.playing
    }

    /**
     * 是否静音状态
     * @returns {*}
     */
    isMute() {
        return this.quieting;
    }

    /**
     *
     * @param value
     */
    setDebug(value) {
        this._opt.isDebug = !!value;
    }

    /**
     *
     * @param value
     */
    setTimeout(value) {
        this._opt.timeout = Number(value);
    }

    /**
     *
     * @param value
     */
    setVod(value) {
        this._opt.vod = !!value;
    }

    /**
     *
     * @param value
     */
    setNoOffscreen(value) {
        this._opt.forceNoOffscreen = !!value;
    }

    /**
     *
     * @param type
     */
    setScaleMode(type) {
        type = Number(type);
        if (type === 0) {
            this._opt.isFullResize = false;
            this._opt.isResize = false;
        } else if (type === 1) {
            this._opt.isFullResize = false;
            this._opt.isResize = true;
        } else if (type === 2) {
            this._opt.isFullResize = true;
        }
        this._resize();
    }

    /**
     *
     */
    mute() {
        this._mute();
    }

    /**
     *
     */
    cancelMute() {
        this._cancelMute();
    }

    /**
     *
     */
    audioResume() {
        this._cancelMute();
    }

    /**
     *
     */
    pause() {
        this._pause();
    }

    /**
     *
     * @param url
     */
    play(url) {
        this._play(url);
    }

    /**
     *
     */
    close() {
        this._close();
    }

    /**
     *
     */
    destroy() {
        this._close();
        this._destroyAudioContext();
        this._destroyContextGL();
        this._decoderWorker.terminate();
        this._removeEventListener();
        this._initCheckVariable();
        this._off();
        this._removeContainerChild();
    }

    /**
     *
     */
    clearView() {
        this._clearView();
    }

    /**
     *
     */
    resize() {
        this._resize();
    }


    /**
     * 设置最大缓冲时长，单位秒，播放器会自动消除延迟
     * @param buffer 单位 秒
     */
    setBufferTime(buffer) {
        buffer = Number(buffer)
        this._decoderWorker.postMessage({
            cmd: POST_MESSAGE.setVideoBuffer,
            time: buffer
        })
    }

    /**
     *
     * @param deg
     */
    setRotate(deg) {
        deg = parseInt(deg, 10)
        const list = [0, 90, 270];
        if (this._opt.rotate === deg || list.indexOf(deg) === -1) {
            return;
        }
        this._opt.rotate = deg;
        this.resize();
    }

    /**
     *
     * @param volume
     */
    setVolume(volume) {
        if (this._gainNode) {
            volume = parseFloat(volume);
            if (isNaN(volume)) {
                return;
            }
            this._gainNode.gain.setValueAtTime(volume, this._audioContext.currentTime);
        }
    }

    /**
     * 开启屏幕常亮, 在play前调用
     * 在手机浏览器上, canvas标签渲染视频并不会像video标签那样保持屏幕常亮
     * H5目前在chrome\edge 84, android chrome 84及以上有原生亮屏API, 需要是https页面
     * 其余平台为模拟实现，此时为兼容实现，并不保证所有浏览器都支持
     */
    setKeepScreenOn() {
        this._opt.keepScreenOn = true;
    }

    /**
     *
     * @param flag
     */
    setFullscreen(flag) {
        const fullscreen = !!flag;
        if (this.fullscreen !== fullscreen) {
            this.fullscreen = fullscreen;
        }
    }


    /**
     *
     * @returns {boolean}
     */
    hasLoaded() {
        return this._hasLoaded;
    }


    /**
     *
     * @param filename
     * @param format
     * @param quality
     * @param type: download,base64,blob
     */
    screenshot(filename, format, quality, type) {
        return this._screenshot(filename, format, quality, type);
    }


    /**
     *
     * @param event
     * @param callback
     */
    on(event, callback) {
        this._on(event, callback);
    }
}

window.Jessibuca = Jessibuca;

export default Jessibuca;
