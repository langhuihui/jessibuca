export function audioContextUnlock(context) {
    context.resume();
    const source = context.createBufferSource();
    source.buffer = context.createBuffer(1, 1, 22050);
    source.connect(context.destination);
    if (source.noteOn)
        source.noteOn(0);
    else
        source.start(0);
}

export function $domToggle($ele, toggle) {
    if ($ele) {
        $ele.style.display = toggle ? 'block' : "none";
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

export function downloadImg(content, fileName) {
    const aLink = document.createElement("a");
    aLink.download = fileName;
    aLink.href = URL.createObjectURL(content);
    aLink.click();
    URL.revokeObjectURL(content);
}

export function bpsSize(value) {
    if (null == value || value === '') {
        return "0 KB/S";
    }
    const srcsize = parseFloat(value);
    let size = srcsize / 1024;
    size = size.toFixed(2);
    return size + 'KB/S';
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

export function setStyle(dom, cssObj) {
    Object.keys(cssObj || {}).forEach(function (key) {
        dom.style[key] = cssObj[key];
    })
}

export function log() {

}

export function checkFull() {
    let isFull = document.fullscreenElement || window.webkitFullscreenElement || document.msFullscreenElement;
    if (isFull === undefined) isFull = false;
    return !!isFull;
}

export function noop() {
}


export function now() {
    return new Date().getTime();
}

export function $hideBtns(doms) {
    Object.keys(doms || {}).forEach((dom) => {
        if (dom !== 'bgDom') {
            $domToggle(doms[dom], false);
        }
    })
}

export function $initBtns($doms) {
    // show
    $domToggle($doms.pauseDom, true);
    $domToggle($doms.screenshotsDom, true);
    $domToggle($doms.fullscreenDom, true);
    $domToggle($doms.quietAudioDom, true);
    $domToggle($doms.textDom, true);
    $domToggle($doms.speedDom, true);
    $domToggle($doms.recordDom, true);
    // hide
    $domToggle($doms.loadingDom, false);
    $domToggle($doms.playDom, false);
    $domToggle($doms.playBigDom, false);
    $domToggle($doms.bgDom, false);
}