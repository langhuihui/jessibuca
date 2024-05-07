import Emitter from "../utils/emitter";
import {CONTROL_HEIGHT, EVENTS, SCREENSHOT_TYPE, VIDEO_ENC_TYPE} from "../constant";
import {dataURLToFile, downloadImg, getBrowser, isAndroid, isMobile, now} from "../utils";
import CommonLoader from "./commonLoader";
import saveAs from "../utils/file-save";

export default class VideoLoader extends CommonLoader {
    constructor(player) {
        super();
        this.player = player;
        const $videoElement = document.createElement('video');
        const $canvasElement = document.createElement('canvas');
        $videoElement.muted = true;
        $videoElement.disablePictureInPicture = true;
        if(isAndroid()){
            // default no poster
            $videoElement.poster = 'noposter';
        }
        $videoElement.style.position = "absolute";
        $videoElement.style.top = 0;
        $videoElement.style.left = 0;
        this._delayPlay = false;
        player.$container.appendChild($videoElement);
        this.videoInfo = {
            width: '',
            height: '',
            encType: '',
        }
        const _opt = this.player._opt;
        if (_opt.useWCS && _opt.wcsUseVideoRender) {
            this.trackGenerator = new MediaStreamTrackGenerator({kind: 'video'});
            $videoElement.srcObject = new MediaStream([this.trackGenerator]);
            this.vwriter = this.trackGenerator.writable.getWriter();
        }
        this.$videoElement = $videoElement;
        this.$canvasElement = $canvasElement;
        this.canvasContext = $canvasElement.getContext('2d');
        this.fixChromeVideoFlashBug();
        this.resize();

        const {proxy} = this.player.events;

        proxy(this.$videoElement, 'canplay', () => {
            this.player.debug.log('Video', `canplay and _delayPlay is ${this._delayPlay}`);
            if (this._delayPlay) {
                this._play();
            }
        })

        proxy(this.$videoElement, 'waiting', () => {
            this.player.emit(EVENTS.videoWaiting);
        })

        proxy(this.$videoElement, 'timeupdate', (event) => {
            // this.player.emit(EVENTS.videoTimeUpdate, event.timeStamp);
            const timeStamp = parseInt(event.timeStamp, 10);
            this.player.emit(EVENTS.timeUpdate, timeStamp)
            // check is pause;
            if (!this.isPlaying() && this.init) {
                this.player.debug.log('Video', `timeupdate and this.isPlaying is false and retry play`);
                this.$videoElement.play();
            }
        })

        this.player.debug.log('Video', 'init');
    }

    destroy() {
        super.destroy();
        this.$canvasElement = null;
        this.canvasContext = null;
        if (this.$videoElement) {
            this.$videoElement.pause();
            this.$videoElement.currentTime = 0;
            this.$videoElement.src = ''
            this.$videoElement.removeAttribute('src');
            this.$videoElement = null;
        }
        if (this.trackGenerator) {
            this.trackGenerator.stop();
            this.trackGenerator = null;
        }
        if (this.vwriter) {
            this.vwriter.close();
            this.vwriter = null;
        }
        this.player.debug.log('Video', 'destroy');
    }

    fixChromeVideoFlashBug() {
        const browser = getBrowser();
        const type = browser.type.toLowerCase();
        if (type === 'chrome' || type === 'edge') {
            const $container = this.player.$container;
            $container.style.backdropFilter = 'blur(0px)';
            $container.style.translateZ = '0';
        }
    }

    play() {
        if (this.$videoElement) {
            const readyState = this._getVideoReadyState();
            this.player.debug.log('Video', `play and readyState: ${readyState}`);
            if (readyState === 0) {
                this.player.debug.warn('Video', 'readyState is 0 and set _delayPlay to true');
                this._delayPlay = true;
                return;
            }
            this._play();
        }
    }

    _getVideoReadyState() {
        let result = 0;
        if (this.$videoElement) {
            result = this.$videoElement.readyState;
        }
        return result;
    }

    _play() {
        this.$videoElement && this.$videoElement.play().then(() => {
            this._delayPlay = false;
            this.player.debug.log('Video', '_play success');
            setTimeout(() => {
                if (!this.isPlaying()) {
                    this.player.debug.warn('Video', `play failed and retry play`)
                    this._play();
                }
            }, 100)

        }).catch((e) => {
            this.player.debug.error('Video', '_play error', e);
        })
    }

    pause(isNow) {
        // 预防
        // https://developer.chrome.com/blog/play-request-was-interrupted/
        // http://alonesuperman.com/?p=23
        if (isNow) {
            this.$videoElement && this.$videoElement.pause();
        } else {
            setTimeout(() => {
                this.$videoElement && this.$videoElement.pause();
            }, 100)
        }
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
        let canvas = this.$canvasElement;
        canvas.width = $video.videoWidth;
        canvas.height = $video.videoHeight;
        this.canvasContext.drawImage($video, 0, 0, canvas.width, canvas.height);
        const dataURL = canvas.toDataURL(formatType[format] || formatType.png, encoderOptions);
        // release memory
        this.canvasContext.clearRect(0, 0, canvas.width, canvas.height);
        canvas.width = 0;
        canvas.height = 0;
        if (type === SCREENSHOT_TYPE.base64) {
            return dataURL;
        } else {
            const file = dataURLToFile(dataURL);
            if (type === SCREENSHOT_TYPE.blob) {
                return file;
            } else if (type === SCREENSHOT_TYPE.download) {
                // downloadImg(file, filename);
                saveAs(file, filename)
            }
        }

    }

    initCanvasViewSize() {
        this.resize();
    }

    //
    render(msg) {
        if (this.vwriter) {
            this.vwriter.write(msg.videoFrame);
        }
    }

    resize() {
        let width = this.player.width;
        let height = this.player.height;
        const option = this.player._opt;
        const rotate = option.rotate;
        if (option.hasControl && !option.controlAutoHide) {
            if (isMobile() && this.player.fullscreen && option.useWebFullScreen) {
                width -= CONTROL_HEIGHT;
            } else {
                height -= CONTROL_HEIGHT;
            }
        }

        this.$videoElement.width = width;
        this.$videoElement.height = height;

        if (rotate === 270 || rotate === 90) {
            this.$videoElement.width = height;
            this.$videoElement.height = width;
        }
        let resizeWidth = this.$videoElement.width;
        let resizeHeight = this.$videoElement.height;
        let left = ((width - resizeWidth) / 2)
        let top = ((height - resizeHeight) / 2)
        let objectFill = 'contain';

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
        this.$videoElement.style.left = left + "px"
        this.$videoElement.style.top = top + "px"
    }

    isPlaying() {
        return this.$videoElement && !this.$videoElement.paused;
    }
}
