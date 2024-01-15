function getBrowser() {
    const UserAgent = window.navigator.userAgent.toLowerCase() || '';
    let browserInfo = {
        type: '',
        version: ''
    };
    var browserArray = {
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
                const versionArray = UserAgent.match(/(msie\s|trident.*rv:)([\w.]+)/)
                if (versionArray && versionArray.length > 2) {
                    versions = UserAgent.match(/(msie\s|trident.*rv:)([\w.]+)/)[2];
                }
            } else if (i === 'Chrome') {
                for (let mt in navigator.mimeTypes) {
                    //检测是否是360浏览器(测试只有pc端的360才起作用)
                    if (navigator.mimeTypes[mt]['type'] === 'application/360softmgrplugin') {
                        i = '360';
                    }
                }
                const versionArray = UserAgent.match(/chrome\/([\d.]+)/);
                if (versionArray && versionArray.length > 1) {
                    versions = versionArray[1];
                }
            } else if (i === 'Firefox') {
                const versionArray = UserAgent.match(/firefox\/([\d.]+)/);
                if (versionArray && versionArray.length > 1) {
                    versions = versionArray[1];
                }
            } else if (i === 'Opera') {
                const versionArray = UserAgent.match(/opera\/([\d.]+)/);
                if (versionArray && versionArray.length > 1) {
                    versions = versionArray[1];
                }
            } else if (i === 'Safari') {
                const versionArray = UserAgent.match(/version\/([\d.]+)/);
                if (versionArray && versionArray.length > 1) {
                    versions = versionArray[1];
                }
            } else if (i === 'Edge') {
                const versionArray = UserAgent.match(/edge\/([\d.]+)/);
                if (versionArray && versionArray.length > 1) {
                    versions = versionArray[1];
                }
            } else if (i === 'QQBrowser') {
                const versionArray = UserAgent.match(/qqbrowser\/([\d.]+)/);
                if (versionArray && versionArray.length > 1) {
                    versions = versionArray[1];
                }
            }
            browserInfo.type = i;
            browserInfo.version = parseInt(versions);
        }
    }
    return browserInfo;
}


function checkSupportMSEHevc() {
    return window.MediaSource && window.MediaSource.isTypeSupported('video/mp4; codecs="hev1.1.6.L123.b0"');
}

function checkSupportMSEH264() {
    return window.MediaSource && window.MediaSource.isTypeSupported('video/mp4; codecs="avc1.64002A"');
}

function checkSupportWCSHevc() {
    const browserInfo = getBrowser();

    return browserInfo.type.toLowerCase() === 'chrome' && browserInfo.version >= 107 && (location.protocol === 'https:' || location.hostname === 'localhost');
}

function checkSupportWCS() {
    return "VideoEncoder" in window;
}

function checkSupportWasm() {
    try {
        if (typeof window.WebAssembly === 'object' && typeof window.WebAssembly.instantiate === 'function') {
            const module = new window.WebAssembly.Module(Uint8Array.of(0x0, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00));
            if (module instanceof window.WebAssembly.Module) {
                return new window.WebAssembly.Instance(module) instanceof window.WebAssembly.Instance;
            }
        }
        return false;
    } catch (e) {
        return false;
    }
}


function checkSupportSIMD() {
    return WebAssembly && WebAssembly.validate(new Uint8Array([0, 97, 115, 109, 1, 0, 0, 0, 1, 5, 1, 96, 0, 1, 123, 3, 2, 1, 0, 10, 10, 1, 8, 0, 65, 0, 253, 15, 253, 98, 11]));
}

function supportSharedArrayBuffer() {
    try {
        new SharedArrayBuffer(1);
        return true;
    } catch (e) {
        return false;
    }
}

let support = document.getElementById('mseSupport');
let notSupport = document.getElementById('mseNotSupport');
if (support && notSupport) {
    if (checkSupportMSEHevc()) {
        support.style.display = 'inline-block'
    } else {
        notSupport.style.display = 'inline-block'
    }
}


let supportH264 = document.getElementById('mseSupport264');
let notSupportH264 = document.getElementById('mseNotSupport264');
if (supportH264 && notSupportH264) {
    if (checkSupportMSEH264()) {
        supportH264.style.display = 'inline-block'
    } else {
        notSupportH264.style.display = 'inline-block'
    }
}


let supportWcsHevc = document.getElementById('wcsSupport');
let notSupportWcsHevc = document.getElementById('wcsNotSupport');

if (supportWcsHevc && notSupportWcsHevc) {
    if (checkSupportWCSHevc()) {
        supportWcsHevc.style.display = 'inline-block';
    } else {
        notSupportWcsHevc.style.display = 'inline-block'
    }
}

let supportWcs = document.getElementById('wcsSupport264');
let notSupportWcs = document.getElementById('wcsNotSupport264');

if (supportWcs && notSupportWcs) {
    if (checkSupportWCS()) {
        supportWcs.style.display = 'inline-block';
    } else {
        notSupportWcs.style.display = 'inline-block'
    }
}

let wasmSupport = document.getElementById('wasmSupport');
let wasmNotSupport = document.getElementById('wasmNotSupport');

if (wasmSupport && wasmNotSupport) {
    if (checkSupportWasm()) {
        wasmSupport.style.display = 'inline-block';
    } else {
        wasmNotSupport.style.display = 'inline-block';
    }
}


let supportSimd = document.getElementById('simdSupport');
let notSupportSimd = document.getElementById('simdNotSupport');

if (supportSimd && notSupportSimd) {
    if (checkSupportSIMD()) {
        supportSimd.style.display = 'inline-block';
    } else {
        notSupportSimd.style.display = 'inline-block'
    }
}

let supportSimdMtSupport = document.getElementById('simdMtSupport');
var notSupportSimdMtSupport = document.getElementById('simdMtNotSupport');


if (supportSimdMtSupport) {
    if (supportSharedArrayBuffer()) {
        supportSimdMtSupport.style.display = 'inline-block';
    } else {
        notSupportSimdMtSupport.style.display = 'inline-block';
    }
}


function isMobile() {
    return (/iphone|ipad|android.*mobile|windows.*phone|blackberry.*mobile/i.test(window.navigator.userAgent.toLowerCase()));
}

function isPad() {
    return (/ipad|android(?!.*mobile)|tablet|kindle|silk/i.test(window.navigator.userAgent.toLowerCase()));
}

const useVconsole = isMobile() || isPad()

if (useVconsole && window.VConsole) {
    new window.VConsole();
}
