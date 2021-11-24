import Emitter from "../utils/emitter";
import {createContextGL, dataURLToFile, downloadImg, now, supportOffscreen} from "../utils";
import createWebGL from "../utils/webgl";
import {CANVAS_RENDER_TYPE, CONTROL_HEIGHT, EVENTS, SCREENSHOT_TYPE, VIDEO_ENC_TYPE} from "../constant";

export default class CanvasVideoLoader extends Emitter {

    constructor(player) {
        super()
        this.player = player;
        const $canvasElement = document.createElement("canvas");
        $canvasElement.style.position = "absolute";
        $canvasElement.style.top = 0;
        $canvasElement.style.left = 0;
        this.$videoElement = $canvasElement;
        player.$container.appendChild(this.$videoElement);

        this.contextGl = null;
        this.contextGlRender = null;
        this.contextGlDestroy = null;
        this.bitmaprenderer = null;
        this.renderType = null;
        this.videoInfo = {
            width: '',
            height: '',
            encType: '',
        }
        //
        this._initCanvasRender();
        this.player.debug.log('CanvasVideo', 'init');
    }

    //
    updateVideoInfo(data) {
        if (data.encTypeCode) {
            this.videoInfo.encType = VIDEO_ENC_TYPE[data.encTypeCode];
        }

        if (data.width) {
            this.videoInfo.width = data.width;
        }

        if (data.height) {
            this.videoInfo.height = data.height;
        }

        // video 基本信息
        if (this.videoInfo.encTypeCode && this.videoInfo.height && this.videoInfo.width) {
            this.player.emit(EVENTS.videoInfo, this.videoInfo);
        }
    }

    _initContextGl() {
        this.contextGl = createContextGL(this.$videoElement);
        const webgl = createWebGL(this.contextGl);
        this.contextGlRender = webgl.render;
        this.contextGlDestroy = webgl.destroy
    }

    initCanvasViewSize() {
        this.$videoElement.width = this.videoInfo.width;
        this.$videoElement.height = this.videoInfo.height;
        this.resize();
    }

    // 渲染类型
    _initCanvasRender() {
        if (this.player._opt.useWCS) {
            this.renderType = CANVAS_RENDER_TYPE.webcodecs;
        } else if (this._supportOffscreen()) {
            this.renderType = CANVAS_RENDER_TYPE.offscreen;
            this._bindOffscreen();
        } else {
            this.renderType = CANVAS_RENDER_TYPE.webgl;
            this._initContextGl();
        }
    }

    _supportOffscreen() {
        return supportOffscreen(this.$videoElement) && this.player._opt.useOffscreen;
    }

    //
    _bindOffscreen() {
        this.bitmaprenderer = this.$videoElement.getContext('bitmaprenderer');
    }

    //
    render(msg) {
        switch (this.renderType) {
            case CANVAS_RENDER_TYPE.offscreen:
                this.bitmaprenderer.transferFromImageBitmap(msg.buffer);
                break;
            case CANVAS_RENDER_TYPE.webgl:
                this.contextGlRender(this.$videoElement.width, this.$videoElement.height, msg.output[0], msg.output[1], msg.output[2]);
                break;
            case CANVAS_RENDER_TYPE.webcodecs:
                this.$videoElement.drawImage(msg.videoFrame, 0, 0, this.$videoElement.width, this.$videoElement.height);
                break;
        }
    }

    screenshot(filename, format, quality, type) {
        filename = filename || now();
        type = type || SCREENSHOT_TYPE.download;
        const formatType = {
            png: 'image/png',
            jpeg: 'image/jpeg',
            webp: 'image/webp'
        };
        let encoderOptions = 0.92;
        if (!formatType[format] && SCREENSHOT_TYPE[format]) {
            type = format;
            format = 'png';
            quality = undefined
        }

        if (typeof quality === "string") {
            type = quality;
            quality = undefined;
        }

        if (typeof quality !== 'undefined') {
            encoderOptions = Number(quality);
        }
        const dataURL = this.$videoElement.toDataURL(formatType[format] || formatType.png, encoderOptions);

        const file = dataURLToFile(dataURL)
        if (type === SCREENSHOT_TYPE.base64) {
            return dataURL;
        } else if (type === SCREENSHOT_TYPE.blob) {
            return file;
        } else if (type === SCREENSHOT_TYPE.download) {
            downloadImg(file, filename);
        }
    }

    //
    clearView() {
        switch (this.renderType) {
            case CANVAS_RENDER_TYPE.offscreen:

                break;
            case CANVAS_RENDER_TYPE.webgl:
                this.contextGl.clear(this.contextGl.COLOR_BUFFER_BIT);
                break;
            case CANVAS_RENDER_TYPE.webcodecs:
                this.$videoElement.clearRect(0, 0, this.$videoElement.width, this.$videoElement.height)
                break;
        }
    }

    resize() {
        this.player.debug.log('canvasVideo', 'resize');
        const width = this.player.width;
        let height = this.player.height;
        if (this.player._opt.hasControl) {
            height -= CONTROL_HEIGHT;
        }
        const resizeWidth = this.$videoElement.width;
        const resizeHeight = this.$videoElement.height;
        const wScale = width / resizeWidth;
        const hScale = height / resizeHeight;
        let scale = wScale > hScale ? hScale : wScale;
        //
        if (!this.player._opt.isResize) {
            if (wScale !== hScale) {
                scale = wScale + ',' + hScale;
            }
        }
        //
        if (this.player._opt.isFullResize) {
            scale = wScale > hScale ? wScale : hScale;
        }
        let transform = "scale(" + scale + ")";
        this.$videoElement.style.transform = transform;
        this.$videoElement.style.left = ((width - resizeWidth) / 2) + "px"
        this.$videoElement.style.top = ((height - resizeHeight) / 2) + "px"
    }

    destroy() {
        if (this.contextGl) {
            this.contextGl = null;
        }
        if (this.contextGlRender) {
            this.contextGlDestroy && this.contextGlDestroy();
            this.contextGlDestroy = null;
            this.contextGlRender = null;
        }
        if (this.bitmaprenderer) {
            this.bitmaprenderer = null;
        }

        this.renderType = null;

        this.videoInfo = {
            width: '',
            height: '',
            encType: '',
            encTypeCode: ''
        }

        this.player.$container.removeChild(this.$videoElement);
        this.player.debug.log(`CanvasVideoLoader`, 'destroy');
    }
}
