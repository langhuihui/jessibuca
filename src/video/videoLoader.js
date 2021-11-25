import Emitter from "../utils/emitter";
import {CONTROL_HEIGHT, EVENTS, SCREENSHOT_TYPE, VIDEO_ENC_TYPE} from "../constant";
import {dataURLToFile, downloadImg, now} from "../utils";

export default class VideoLoader extends Emitter {
    constructor(player) {
        super();
        this.player = player;
        const $videoElement = document.createElement('video');
        $videoElement.muted = true;
        $videoElement.style.position = "absolute";
        $videoElement.style.top = 0;
        $videoElement.style.left = 0;
        player.$container.appendChild($videoElement);
        this.$videoElement = $videoElement;
        this.videoInfo = {
            width: '',
            height: '',
            encType: '',
        }
        this.resize();
        this.player.debug.log('Video', 'init');
    }

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

    play() {
        this.$videoElement.play();
    }

    clearView() {

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
        const $video = this.$videoElement;
        let canvas = document.createElement('canvas');
        canvas.width = $video.videoWidth;
        canvas.height = $video.videoHeight;
        const context = canvas.getContext('2d');
        context.drawImage($video, 0, 0, canvas.width, canvas.height);
        const dataURL = canvas.toDataURL(SCREENSHOT_TYPE[format] || SCREENSHOT_TYPE.png, encoderOptions);
        const file = dataURLToFile(dataURL)
        if (type === SCREENSHOT_TYPE.base64) {
            return dataURL;
        } else if (type === SCREENSHOT_TYPE.blob) {
            return file;
        } else if (type === SCREENSHOT_TYPE.download) {
            downloadImg(file, filename);
        }
    }


    resize() {
        this.$videoElement.width = this.player.width;
        this.$videoElement.height = this.player._opt.hasControl ? this.player.height - CONTROL_HEIGHT : this.player.height;
    }

    destroy() {
        this.player.$container.removeChild(this.$videoElement);
        this.player.debug.log('Video', 'destroy');
        this.player = null;
    }

}
