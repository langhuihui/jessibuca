import initVideo from "./video/canvas";
import initAudio from "./audio/audioContext";
import initDashboard from './dashboard/dashboard';
import initDecodeWorker from './worker/decodeWorker';
import initMethods from './core/methods';
import initStatus from './core/status';
import {DEFAULT_OPTIONS, POST_MESSAGE} from "./constant";
import {$domToggle, checkFull, dataURLToFile, downloadImg, now} from "./utils";


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

        this._stats = {
            buf: 0, //ms
            fps: 0,
            abps: '',
            vbps: '',
            ts: ''
        }

        this._hasLoaded = false;
        this._playUrl = '';

        //
        initStatus(this);
        //
        initMethods(this);
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
            this._trigger('fullscreen', value);
        }


        this._fullscreen = value;
    }

    get fullscreen() {

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

    _close(){

    }


    destroy() {
        this._close();
    }

    clearView() {


    }

    play(url) {
        if (!this._playUrl && !url) {
            return;
        }
        let needDelay = false;
        if (url) {
            if (this.playUrl) {
                this._close();
                needDelay = true;
                this.clearView();
            }
            this.loading = true;
            _domToggle(this._doms.bgDom, false);
            this._checkLoading();
            this.playUrl = url;
        } else if (this.playUrl) {
            // retry
            if (this.loading) {
                this._hideBtns();
                _domToggle(this._doms.fullscreenDom, true);
                _domToggle(this._doms.pauseDom, true);
                _domToggle(this._doms.loadingDom, true);
                this._checkLoading();
            } else {
                this.playing = true;
            }
        }
        this._initCheckVariable();

        if (needDelay) {
            var _this = this;
            setTimeout(function () {
                _this._decoderWorker.postMessage({ cmd: "play", url: _this.playUrl })
            }, 300);
        } else {
            this._decoderWorker.postMessage({ cmd: "play", url: this.playUrl })
        }
    }

    resize() {
        var width = this.$container.clientWidth;
        var height = this.$container.clientHeight;
        if (this._showControl()) {
            height -= 38;
        }
        var resizeWidth = this.$canvasElement.width;
        var resizeHeight = this.$canvasElement.height;
        var rotate = this._opt.rotate;
        var wScale = width / resizeWidth;
        var hScale = height / resizeHeight;
        var scale = wScale > hScale ? hScale : wScale;
        if (!this._opt.isResize) {
            if (wScale !== hScale) {
                scale = wScale + ',' + hScale;
            }
        }
        //
        if (this._opt.isFullResize) {
            scale = wScale > hScale ? wScale : hScale;
        }

        let transform = "scale(" + scale + ")";

        if (rotate) {
            transform += ' rotate(' + rotate + 'deg)'
        }

        this.$canvasElement.style.transform = transform;
        this.$canvasElement.style.left = ((width - resizeWidth) / 2) + "px"
        this.$canvasElement.style.top = ((height - resizeHeight) / 2) + "px"
    }


    setBufferTime(buffer) {
        buffer = Number(buffer)
        this._stats.buf = buffer * 1000;
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
     *
     * @returns {boolean}
     */
    hasLoaded() {
        return this._hasLoaded;
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
     * @param filename
     * @param format
     * @param quality
     */
    screenshot(filename, format, quality) {
        filename = filename || now();
        const formatType = {
            png: 'image/png',
            jpeg: 'image/jpeg',
            webp: 'image/webp'
        };
        let encoderOptions = 0.92;
        if (typeof quality !== 'undefined') {
            encoderOptions = Number(quality);
        }
        const dataURL = this.$canvasElement.toDataURL(formatType[format] || formatType.png, encoderOptions);
        downloadImg(dataURLToFile(dataURL), filename);
    }

    on(events, callback) {
        let cache, event, list;
        if (!callback) return this;
        cache = this.__events || (this.__events = {});
        events = events.split(/\s+/);
        while (event = events.shift()) {
            list = cache[event] || (cache[event] = []);
            list.push(callback);
        }
        return this;
    }

    _off() {
        let cache;
        if (!(cache = this.__events)) return this;
        delete this.__events;
        return this;
    }

    _trigger(events) {

        // Execute callbacks
        function _callEach(list, args, context) {
            if (list) {
                for (let i = 0, len = list.length; i < len; i += 1) {
                    list[i].apply(context, args);
                }
            }
        }

        const eventSplitter = /\s+/;
        let cache, event, all, list, i, len, rest = [], args;
        if (!(cache = this.__events)) return this;
        events = events.split(/\s+/);
        // Fill up `rest` with the callback arguments.  Since we're only copying
        // the tail of `arguments`, a loop is much faster than Array#slice.
        for (i = 1, len = arguments.length; i < len; i++) {
            rest[i - 1] = arguments[i];
        }
        // For each event, walk through the list of callbacks twice, first to
        // trigger the event, then to trigger any `"all"` callbacks.
        while (event = events.shift()) {
            if (list = cache[event]) list = list.slice();
            // Execute event callbacks.
            _callEach(list, rest, this);
        }
        return this;
    }

}