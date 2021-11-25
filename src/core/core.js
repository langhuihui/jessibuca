import {
    $domToggle,
    $hideBtns,
    bufferStatus,
    checkFull,
    createEmptyImageBitmap,
    dataURLToFile,
    downloadImg,
    fpsStatus,
    now
} from "../utils";
import {EVEMTS, POST_MESSAGE, SCREENSHOT_TYPE} from "../constant";

export default (jessibuca) => {
    //
    jessibuca._pause = () => {
        jessibuca._close();
        if (jessibuca.loading) {
            $domToggle(jessibuca.$doms.loadingDom, false);
        }
        jessibuca.recording = false;
        jessibuca.playing = false;
    }


    jessibuca._play = (url) => {
        if (!jessibuca._playUrl && !url) {
            return;
        }
        let needDelay = false;
        if (url) {
            if (jessibuca._playUrl) {
                jessibuca._close();
                needDelay = true;
                jessibuca.clearView();
            }
            jessibuca.loading = true;
            $domToggle(jessibuca.$doms.bgDom, false);
            jessibuca._checkLoading();
            jessibuca._playUrl = url;
        } else if (jessibuca._playUrl) {
            // retry
            if (jessibuca.loading) {
                $hideBtns(jessibuca.$doms);
                $domToggle(jessibuca.$doms.fullscreenDom, true);
                $domToggle(jessibuca.$doms.pauseDom, true);
                $domToggle(jessibuca.$doms.loadingDom, true);
                jessibuca._checkLoading();
            } else {
                jessibuca.playing = true;
            }
        }
        jessibuca._initCheckVariable();

        if (needDelay) {
            setTimeout(() => {
                jessibuca._decoderWorker.postMessage({cmd: POST_MESSAGE.play, url: jessibuca._playUrl})
            }, 300);
        } else {
            jessibuca._decoderWorker.postMessage({cmd: POST_MESSAGE.play, url: jessibuca._playUrl})
        }
    }


    jessibuca._screenshot = (filename, format, quality, type) => {
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
        const dataURL = jessibuca.$canvasElement.toDataURL(formatType[format] || formatType.png, encoderOptions);
        const file = dataURLToFile(dataURL)
        if (type === SCREENSHOT_TYPE.base64) {
            return dataURL;
        } else if (type === SCREENSHOT_TYPE.blob) {
            return file;
        } else if (type === SCREENSHOT_TYPE.download) {
            downloadImg(file, filename);
        }
    }

    jessibuca._close = () => {
        jessibuca._close$2();
        jessibuca._clearView();
    }

    jessibuca._close$2 = () => {
        // jessibuca._opt.debug && console.log('_close$2-START');
        jessibuca._closeAudio && jessibuca._closeAudio()
        jessibuca._audioPlayBuffers = [];
        jessibuca._audioPlaying = false;
        jessibuca._decoderWorker.postMessage({cmd: POST_MESSAGE.close})
        delete jessibuca._playAudio;
        jessibuca._releaseWakeLock();
        jessibuca._initCheckVariable();
        // jessibuca._opt.debug && console.log('_close$2-END');
    }

    jessibuca._releaseWakeLock = () => {
        if (jessibuca._wakeLock) {
            jessibuca._wakeLock.release();
            jessibuca._wakeLock = null;
        }
    }


    jessibuca._clearView = () => {
        if (jessibuca._contextGL) {
            jessibuca._contextGL.clear(jessibuca._contextGL.COLOR_BUFFER_BIT);
        } else if (jessibuca._bitmaprenderer) {
            createEmptyImageBitmap(jessibuca.$canvasElement.width, jessibuca.$canvasElement.height).then((imageBitMap) => {
                jessibuca._bitmaprenderer.transferFromImageBitmap(imageBitMap);
            })
        }
    }

    jessibuca._resize = () => {
        let width = jessibuca.$container.clientWidth;
        let height = jessibuca.$container.clientHeight;
        if (jessibuca._showControl()) {
            height -= 38;
        }
        let resizeWidth = jessibuca.$canvasElement.width;
        let resizeHeight = jessibuca.$canvasElement.height;
        const rotate = jessibuca._opt.rotate;
        let left = ((width - resizeWidth) / 2)
        let top = ((height - resizeHeight) / 2)
        if (rotate === 270 || rotate === 90) {
            resizeWidth = jessibuca.$canvasElement.height;
            resizeHeight = jessibuca.$canvasElement.width;
        }
        let wScale = width / resizeWidth;
        let hScale = height / resizeHeight;


        let scale = wScale > hScale ? hScale : wScale;
        if (!jessibuca._opt.isResize) {
            if (wScale !== hScale) {
                scale = wScale + ',' + hScale;
            }
        }
        //
        if (jessibuca._opt.isFullResize) {
            scale = wScale > hScale ? wScale : hScale;
        }

        let transform = "scale(" + scale + ")";

        if (rotate) {
            transform += ' rotate(' + rotate + 'deg)'
        }


        jessibuca.$canvasElement.style.transform = transform;
        jessibuca.$canvasElement.style.left = left + "px"
        jessibuca.$canvasElement.style.top = top + "px"
    }


    jessibuca._enableWakeLock = () => {
        if (jessibuca._opt.keepScreenOn) {
            if ("wakeLock" in navigator) {
                navigator.wakeLock.request("screen").then((lock) => {
                    jessibuca._wakeLock = lock;
                })
            }
        }
    }

    jessibuca._supportOffscreen = () => {
        return !jessibuca._opt.forceNoOffscreen && typeof jessibuca.$canvasElement.transferControlToOffscreen == 'function'
    }


    jessibuca._checkHeart = () => {
        jessibuca._clearCheckHeartTimeout();
        jessibuca._startCheckHeartTimeout();
    }

    jessibuca._updateStats = (options) => {
        options = options || {};

        if (!jessibuca._startBpsTime) {
            jessibuca._startBpsTime = now();
        }
        const _nowTime = now();
        const timestamp = _nowTime - jessibuca._startBpsTime;

        if (timestamp < 1 * 1000) {
            jessibuca._stats.fps += 1;
            return;
        }
        jessibuca._stats.ts = options.ts;
        jessibuca._stats.buf = options.buf;
        jessibuca._trigger(EVEMTS.stats, jessibuca._stats);
        jessibuca._trigger(EVEMTS.performance, fpsStatus(jessibuca._stats.fps));
        jessibuca._trigger(EVEMTS.buffer, bufferStatus(jessibuca._stats.buf, jessibuca._opt.videoBuffer * 1000));
        jessibuca._stats.fps = 0;
        jessibuca._startBpsTime = _nowTime;
    }

    //
    jessibuca._onfullscreenchange = () => {
        jessibuca.fullscreen = checkFull();
    }
    //
    jessibuca._handleVisibilityChange = () => {
        if (jessibuca._opt.hiddenAutoPause) {
            jessibuca._opt.debug && console.log(document.visibilityState, jessibuca._isPlayingBeforePageHidden);
            // show
            if ("visible" === document.visibilityState) {
                if (jessibuca._isPlayingBeforePageHidden) {
                    jessibuca._play();
                }
            } else {
                jessibuca._isPlayingBeforePageHidden = jessibuca.playing;
                // hidden
                if (jessibuca.playing) {
                    jessibuca._pause();
                }
            }
        }
    }

    jessibuca._handleWakeLock = () => {
        if (jessibuca._wakeLock !== null && "visible" === document.visibilityState) {
            jessibuca._enableWakeLock();
        }
    }


    jessibuca._reset = () => {
        jessibuca.recording = false;
        jessibuca.playing = false;
        jessibuca.loading = false;
        jessibuca._clearCheckLoading();
        jessibuca._close();
    }
}