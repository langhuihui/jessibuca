import Emitter from "../utils/emitter";
import {CONTROL_HEIGHT, EVENTS, SCREENSHOT_TYPE, VIDEO_ENC_TYPE} from "../constant";
import {dataURLToFile, downloadImg, now} from "../utils";
import CommonLoader from "./commonLoader";

export default class VideoLoader extends CommonLoader {
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

        const {proxy} = this.player.events;

        proxy(this.$videoElement, 'canplay', () => {
            this.player.debug.log('Video', 'canplay');
        })

        proxy(this.$videoElement, 'waiting', () => {
            this.player.emit(EVENTS.videoWaiting);
        })

        proxy(this.$videoElement, 'timeupdate', (event) => {
            // this.player.emit(EVENTS.videoTimeUpdate, event.timeStamp);
        })

        this.player.debug.log('Video', 'init');

    }

    play() {
        // this.$videoElement.autoplay = true;
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

    initCanvasViewSize() {
        this.resize();
    }

    resize() {
        this.$videoElement.width = this.player.width;
        this.$videoElement.height = this.player._opt.hasControl ? this.player.height - CONTROL_HEIGHT : this.player.height;
        const option = this.player._opt;
        let objectFill = 'contain';
        const rotate = option.rotate;

        // 默认是true
        // 视频画面做等比缩放后,高或宽对齐canvas区域,画面不被拉伸,但有黑边

        // 视频画面完全填充canvas区域,画面会被拉伸
        if (!option.isResize) {
            objectFill = 'fill';
        }

        // 视频画面做等比缩放后,完全填充canvas区域,画面不被拉伸,没有黑边,但画面显示不全
        if (option.isFullResize) {
            objectFill = 'none';
        }
        this.$videoElement.style.objectFit = objectFill;
        this.$videoElement.style.transform = 'rotate(' + rotate + 'deg)';
    }


    destroy() {
        this.player.$container.removeChild(this.$videoElement);
        this.init = false;
        this.off();
        this.player.debug.log('Video', 'destroy');
    }

}
