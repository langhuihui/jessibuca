import {MP4_CODECS, FILE_SUFFIX} from "../constant";
import screenfull from "screenfull";

export function noop() {
}


export function supportOffscreen($canvas) {
    return typeof $canvas.transferControlToOffscreen === 'function';
}


export function supportOffscreenV2() {
    return typeof OffscreenCanvas !== "undefined";
}


export function createContextGL($canvas) {
    let gl = null;

    const validContextNames = ["webgl", "experimental-webgl", "moz-webgl", "webkit-3d"];
    let nameIndex = 0;

    while (!gl && nameIndex < validContextNames.length) {
        const contextName = validContextNames[nameIndex];

        try {
            let contextOptions = {preserveDrawingBuffer: true};
            gl = $canvas.getContext(contextName, contextOptions);
        } catch (e) {
            gl = null;
        }

        if (!gl || typeof gl.getParameter !== "function") {
            gl = null;
        }

        ++nameIndex;
    }


    return gl;
}


export function audioContextUnlock(context) {
    context.resume();
    const source = context.createBufferSource();
    source.buffer = context.createBuffer(1, 1, 22050);
    source.connect(context.destination);
    if (source.noteOn) {
        source.noteOn(0);
    } else {
        source.start(0);
    }
}

export function dataURLToFile(dataURL = '') {
    const arr = dataURL.split(",");
    const bstr = atob(arr[1]);
    const type = arr[0].replace("data:", "").replace(";base64", "")
    let n = bstr.length, u8arr = new Uint8Array(n);
    while (n--) {
        u8arr[n] = bstr.charCodeAt(n);
    }
    return new File([u8arr], 'file', {type});
}

export function downloadFile(file, fileName) {
    if (file instanceof Blob || file instanceof File) {
        file = new Blob([file]);
    }
    const aLink = document.createElement("a");
    aLink.download = fileName;
    aLink.href = file;
    aLink.click();
}


export function downloadImg(content, fileName) {
    const aLink = document.createElement("a");
    aLink.download = fileName;
    const href = URL.createObjectURL(content);
    aLink.href = href;
    aLink.click();
    setTimeout(() => {
        URL.revokeObjectURL(href);
    }, isIOS() ? 1000 : 0)
}

export function checkFull() {
    let isFull = document.fullscreenElement || window.webkitFullscreenElement || document.msFullscreenElement;
    if (isFull === undefined) isFull = false;
    return !!isFull;
}

export function now() {
    return new Date().getTime();
}

export const supportedWasm = (() => {
    try {
        if (typeof WebAssembly === "object"
            && typeof WebAssembly.instantiate === "function") {
            const module = new WebAssembly.Module(Uint8Array.of(0x0, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00));
            if (module instanceof WebAssembly.Module)
                return new WebAssembly.Instance(module) instanceof WebAssembly.Instance;
        }
    } catch (e) {
    }
    return false;
})();

export function clamp(num, a, b) {
    return Math.max(Math.min(num, Math.max(a, b)), Math.min(a, b));
}

export function setStyle(element, key, value) {
    if (!element) {
        return
    }
    if (typeof key === 'object') {
        Object.keys(key).forEach(item => {
            setStyle(element, item, key[item]);
        });
    }
    element.style[key] = value;
    return element;
}


export function getStyle(element, key, numberType = true) {
    if (!element) {
        return 0
    }

    const value = getComputedStyle(element, null).getPropertyValue(key);
    return numberType ? parseFloat(value) : value;
}

export function getNowTime() {
    if (performance && typeof performance.now === 'function') {
        return performance.now();
    }
    return Date.now();
}

export function calculationRate(callback) {
    let totalSize = 0;
    let lastTime = getNowTime();
    return size => {
        totalSize += size;
        const thisTime = getNowTime();
        const diffTime = thisTime - lastTime;
        if (diffTime >= 1000) {
            callback((totalSize / diffTime) * 1000);
            lastTime = thisTime;
            totalSize = 0;
        }
    };
}

export function downloadRecord(blob, name, suffix) {
    const url = window.URL.createObjectURL(blob);
    const a = document.createElement('a');
    a.href = url;
    a.download = (name || now()) + '.' + (suffix || FILE_SUFFIX.webm);
    a.click();
    setTimeout(() => {
        window.URL.revokeObjectURL(url);
    }, isIOS() ? 1000 : 0)
}

export const env = '__ENV__';

export function isMobile() {
    return (/iphone|ipod|android.*mobile|windows.*phone|blackberry.*mobile/i.test(window.navigator.userAgent.toLowerCase()));
}

export function isAndroid() {
    const UA = window.navigator.userAgent.toLowerCase();
    return (/android/i.test(UA));
}

export function isIOS() {
    const UA = window.navigator.userAgent.toLowerCase();
    return UA && /iphone|ipad|ipod|ios/.test(UA);
}


export function parseTime(time, cFormat) {
    if (arguments.length === 0) {
        return null
    }
    var format = cFormat || '{y}-{m}-{d} {h}:{i}:{s}'
    var date;
    if (typeof time === 'object') {
        date = time
    } else {
        if (('' + time).length === 10) time = parseInt(time) * 1000;
        time = +time; // 转成int 型
        date = new Date(time)
    }
    var formatObj = {
        y: date.getFullYear(),
        m: date.getMonth() + 1,
        d: date.getDate(),
        h: date.getHours(),
        i: date.getMinutes(),
        s: date.getSeconds(),
        a: date.getDay()
    };
    var time_str = format.replace(/{(y|m|d|h|i|s|a)+}/g, (result, key) => {
        var value = formatObj[key]
        if (key === 'a') return ['一', '二', '三', '四', '五', '六', '日'][value - 1]
        if (result.length > 0 && value < 10) {
            value = '0' + value
        }
        return value || 0
    });
    return time_str
}

// 是否支持 webcodecs
export function supportWCS() {
    return "VideoEncoder" in window;
}

export function toNumber(value) {
    if (typeof value !== 'string') {
        return value;
    } else {
        // 转换成 number 类型
        var parsed = Number(value);
        return isNaN(parsed) ? value : parsed;
    }
}

export function uuid16() {
    return 'xxxxxxxxxxxx4xxx'.replace(/[xy]/g, function (c) {
        var r = Math.random() * 16 | 0, v = c == 'x' ? r : (r & 0x3 | 0x8)
        return v.toString(16)
    })
}

export function throttle(callback, delay) {
    let isThrottled = false;
    let args;
    let context;

    function fn(...args2) {
        if (isThrottled) {
            args = args2;
            context = this;
            return;
        }

        isThrottled = true;
        callback.apply(this, args2);
        setTimeout(() => {
            isThrottled = false;
            if (args) {
                fn.apply(context, args);
                args = null;
                context = null;
            }
        }, delay);
    }

    return fn;
}


export function isDef(v) {
    return v !== undefined && v !== null;
}

export function formatVideoDecoderConfigure(avcC) {
    let codecArray = avcC.subarray(1, 4);
    let codecString = "avc1.";
    for (let j = 0; j < 3; j++) {
        let h = codecArray[j].toString(16);
        if (h.length < 2) {
            h = "0" + h
        }
        codecString += h
    }

    return {
        codec: codecString,
        description: avcC
    }
}

export function isFullScreen() {
    return screenfull.isFullscreen
}

export function bpsSize(value) {
    if (null == value || value === '' || parseInt(value) === 0 || isNaN(parseInt(value))) {
        return "0KB/s";
    }
    let size = parseFloat(value);
    size = size.toFixed(2);
    return size + 'KB/s';
}


export function fpsStatus(fps) {
    let result = 0;
    if (fps >= 24) {
        result = 2;
    } else if (fps >= 15) {
        result = 1;
    }

    return result;
}

export function createEmptyImageBitmap(width, height) {
    const $canvasElement = document.createElement("canvas");
    $canvasElement.width = width;
    $canvasElement.height = height;
    return window.createImageBitmap($canvasElement, 0, 0, width, height);
}


export function supportMSE() {
    return window.MediaSource && window.MediaSource.isTypeSupported(MP4_CODECS.avc);
}

export function supportMediaStreamTrack() {
    return window.MediaStreamTrackGenerator && typeof window.MediaStreamTrackGenerator === 'function'
}


export function formatMp4VideoCodec(codec) {
    return `video/mp4; codecs="${codec}"`
}


export function saveBlobToFile(fileName, blob) {
    let url = window.URL.createObjectURL(blob);
    let aLink = window.document.createElement('a');
    aLink.download = fileName;
    aLink.href = url;
    //创建内置事件并触发
    let evt = window.document.createEvent('MouseEvents');
    evt.initEvent("click", true, true); //initEvent 不加后两个参数在FF下会报错  事件类型，是否冒泡，是否阻止浏览器的默认行为
    aLink.dispatchEvent(evt);
    setTimeout(() => {
        window.URL.revokeObjectURL(url);
    }, isIOS() ? 1000 : 0)
}

export function isEmpty(value) {
    return value === null || value === undefined
}

export function isBoolean(value) {
    return value === true || value === false;
}

export function isNotEmpty(value) {
    return !isEmpty(value)
}

export function initPlayTimes() {
    return {
        playInitStart: '', //1
        playStart: '', // 2
        streamStart: '', //3
        streamResponse: '', // 4
        demuxStart: '', // 5
        decodeStart: '', // 6
        videoStart: '', // 7
        playTimestamp: '',// playStart- playInitStart
        streamTimestamp: '',// streamStart - playStart
        streamResponseTimestamp: '',// streamResponse - streamStart
        demuxTimestamp: '', // demuxStart - streamResponse
        decodeTimestamp: '', // decodeStart - demuxStart
        videoTimestamp: '',// videoStart - decodeStart
        allTimestamp: '' // videoStart - playInitStart
    }
}

// create watermark
export function createWatermark(options) {
    let defaultConfig = {
        container: '',
        left: '',
        right: '',
        top: '',
        bottom: '',
        image: {
            src: '',
            width: '100',
            height: '60',
        },
        text: {
            content: '',
            fontSize: '14',
            color: '#000'
        },
    }
    defaultConfig = Object.assign(defaultConfig, options)

    const $container = defaultConfig.container;

    if ($container) {
        return
    }

    let shadowRoot = null;
    const otDiv = document.createElement('div');
    otDiv.setAttribute('style', 'pointer-events: none !important; display: block !important');

    if (typeof otDiv.attachShadow === "function") {
        shadowRoot = otDiv.attachShadow({mode: 'open'});
    } else if (otDiv.shadowRoot) {
        shadowRoot = otDiv.shadowRoot;
    } else {
        shadowRoot = otDiv;
    }

    const nodeList = $container.children;
    const index = Math.floor(Math.random() * (nodeList.length - 1));

    if (nodeList[index]) {
        $container.insertBefore(otDiv, nodeList[index]);
    } else {
        $container.appendChild(otDiv);
    }


    const maskDiv = document.createElement('div');
    let innerDom = null;
    if (defaultConfig.text && defaultConfig.text.content) {
        innerDom = document.createTextNode(defaultConfig.text);
    } else if (defaultConfig.image && defaultConfig.image.src) {
        innerDom = document.createElement('img')
        innerDom.style.height = '100%'
        innerDom.style.width = '100%'
        innerDom.src = defaultConfig.image.src;
    }

    if (!innerDom) {
        return;
    }

    maskDiv.appendChild(innerDom);

    maskDiv.style.visibility = '';
    maskDiv.style.position = "absolute";
    maskDiv.style.display = 'block'
    maskDiv.style['-ms-user-select'] = "none";
    maskDiv.style.left = defaultConfig.left;
    maskDiv.style.right = defaultConfig.right;
    maskDiv.style.top = defaultConfig.top;
    maskDiv.style.bottom = defaultConfig.bottom;
    maskDiv.style.overflow = 'hidden';
    maskDiv.style.zIndex = "9999999";
    if (defaultConfig.text && defaultConfig.text.content) {
        maskDiv.style.fontSize = defaultConfig.text.fontSize;
        maskDiv.style.color = defaultConfig.text.color;
    } else if (defaultConfig.image && defaultConfig.image.src) {
        maskDiv.style.width = defaultConfig.image.width + 'px';
        maskDiv.style.height = defaultConfig.image.height + 'px';
    }

    shadowRoot.appendChild(maskDiv)

    // remove function
    return () => {
        $container.removeChild(otDiv);
    }
}

export function formatTimeTips(time) {
    var result;

    //
    if (time > -1) {
        var hour = Math.floor(time / 3600);
        var min = Math.floor(time / 60) % 60;
        var sec = time % 60;

        sec = Math.round(sec);

        if (hour < 10) {
            result = '0' + hour + ":";
        } else {
            result = hour + ":";
        }

        if (min < 10) {
            result += "0";
        }
        result += min + ":";
        if (sec < 10) {
            result += "0";
        }
        result += sec.toFixed(0);
    }

    return result;
}

export function getTarget(e) {
    const event = e || window.event;
    const target = event.target || event.srcElement;
    return target;
}

export function isWebglRenderSupport(width) {
    return (width / 2) % 4 === 0
}

export function isGreenYUV(arrayBuffer) {
    let zeroNum = 0;
    for (let i = 0; i < 10; i++) {
        let temp = arrayBuffer[i];
        if (temp === 0) {
            zeroNum += 1
        }
    }
    return zeroNum === 10;
}

export function getBrowser() {
    const UserAgent = navigator.userAgent.toLowerCase();
    const browserInfo = {};
    const browserArray = {
        IE: window.ActiveXObject || "ActiveXObject" in window, // IE
        Chrome: UserAgent.indexOf('chrome') > -1 && UserAgent.indexOf('safari') > -1, // Chrome浏览器
        Firefox: UserAgent.indexOf('firefox') > -1, // 火狐浏览器
        Opera: UserAgent.indexOf('opera') > -1, // Opera浏览器
        Safari: UserAgent.indexOf('safari') > -1 && UserAgent.indexOf('chrome') == -1, // safari浏览器
        Edge: UserAgent.indexOf('edge') > -1, // Edge浏览器
        QQBrowser: /qqbrowser/.test(UserAgent), // qq浏览器
        WeixinBrowser: /MicroMessenger/i.test(UserAgent) // 微信浏览器
    };
    // console.log(browserArray)
    for (let i in browserArray) {
        if (browserArray[i]) {
            let versions = '';
            if (i === 'IE') {
                versions = UserAgent.match(/(msie\s|trident.*rv:)([\w.]+)/)[2];
            } else if (i === 'Chrome') {
                for (let mt in navigator.mimeTypes) {
                    //检测是否是360浏览器(测试只有pc端的360才起作用)
                    if (navigator.mimeTypes[mt]['type'] === 'application/360softmgrplugin') {
                        i = '360';
                    }
                }
                versions = UserAgent.match(/chrome\/([\d.]+)/)[1];
            } else if (i === 'Firefox') {
                versions = UserAgent.match(/firefox\/([\d.]+)/)[1];
            } else if (i === 'Opera') {
                versions = UserAgent.match(/opera\/([\d.]+)/)[1];
            } else if (i === 'Safari') {
                versions = UserAgent.match(/version\/([\d.]+)/)[1];
            } else if (i === 'Edge') {
                versions = UserAgent.match(/edge\/([\d.]+)/)[1];
            } else if (i === 'QQBrowser') {
                versions = UserAgent.match(/qqbrowser\/([\d.]+)/)[1];
            }
            browserInfo.type = i;
            browserInfo.version = parseInt(versions);
        }
    }
    return browserInfo;
}


export function closeVideoFrame(videoFrame) {
    if (videoFrame.close) {
        videoFrame.close()
    } else if (videoFrame.destroy) {
        videoFrame.destroy()
    }
}


export function removeElement(element) {
    let result = false;
    if (element) {
        if (element.parentNode) {
            element.parentNode.removeChild(element);
            result = true;
        }
    }
    return result;
}

export function hevcEncoderNalePacketNotLength(oneNALBuffer, isIframe) {
    const idrBit = 0x10 | 12;
    const nIdrBit = 0x20 | 12;
    let tmp = [];
    if (isIframe) {
        tmp[0] = idrBit;
    } else {
        tmp[0] = nIdrBit;
    }
    tmp[1] = 1;
    //
    tmp[2] = 0;
    tmp[3] = 0;
    tmp[4] = 0;

    const arrayBuffer = new Uint8Array(tmp.length + oneNALBuffer.byteLength);
    arrayBuffer.set(tmp, 0);
    arrayBuffer.set(oneNALBuffer, tmp.length);
    return arrayBuffer;
}
