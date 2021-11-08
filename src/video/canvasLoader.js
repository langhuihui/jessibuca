import Emitter from "../utils/emitter";
import {createContextGL, dataURLToFile, downloadImg, now, supportOffscreen} from "../utils";
import createWebGL from "../utils/webgl";
import {SCREENSHOT_TYPE} from "../constant";


export default class CanvasVideoLoader extends Emitter {

    constructor(player) {
        super()
        this.player = player;
        const $canvasElement = document.createElement("canvas");
        $canvasElement.style.position = "absolute";
        $canvasElement.style.top = 0;
        $canvasElement.style.left = 0;
        this.$videoElement = $canvasElement;
        this.contextGl = null;
        this.contextGlRender = null;
        this.contextGlDestroy = null;
        this.bitmaprenderer = null;
        this.videoInfo = {
            width: '',
            height: '',
            encType: '',
            encTypeCode: ''
        }
    }

    _initContextGl() {
        this.contextGl = createContextGL(this.$videoElement);
        const webgl = createWebGL(this.contextGl);
        this.contextGlRender = webgl.render;
        this.contextGlDestroy = webgl.destroy
    }

    _initCanvasViewSize() {
        this.$videoElement.width = this.videoInfo.width;
        this.$videoElement.height = this.videoInfo.height;
        this.resize();
    }

    _supportOffscreen() {
        return supportOffscreen(this.$videoElement) && !this.player._opt.forceNoOffscreen;
    }

    //
    bindOffscreen() {
        if (this._supportOffscreen()) {
            this.bitmaprenderer = this.$videoElement.getContext('bitmaprenderer');
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


    destroy() {
        if (this.contextGl) {
            this.contextGl = null;
        }
        if (this.contextGlRender) {
            this.contextGlDestroy && this.contextGlDestroy();
            this.contextGlRender = null;
        }
        if (this.bitmaprenderer) {
            this.bitmaprenderer = null;
        }

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
