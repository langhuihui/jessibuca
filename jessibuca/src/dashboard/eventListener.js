import {now} from "../utils";

export default (jessibuca) => {
    jessibuca._resize$2 = () => jessibuca.resize();

    jessibuca._onfullscreenchange$2 = () => jessibuca._onfullscreenchange();

    window.addEventListener('resize', jessibuca._resize$2);
    window.addEventListener('fullscreenchange', jessibuca._onfullscreenchange$2);

    jessibuca._handleWakeLock = () => {
        if (jessibuca._wakeLock !== null && "visible" === document.visibilityState) {
            jessibuca._enableWakeLock();
        }
    }

    document.addEventListener('visibilitychange', jessibuca._handleWakeLock);
    window.addEventListener('fullscreenchange', jessibuca._handleWakeLock);


    if (jessibuca._opt.supportDblclickFullscreen) {
        jessibuca.$canvasElement.addEventListener('dblclick', () => {
            jessibuca.fullscreen = !jessibuca.fullscreen;
        }, false);
    }

    jessibuca._removeEventListener = () => {
        window.removeEventListener('resize', jessibuca._resize$2);
        window.removeEventListener('fullscreenchange', jessibuca._onfullscreenchange$2);
        document.removeEventListener('visibilitychange', jessibuca._handleWakeLock);
        window.removeEventListener('fullscreenchange', jessibuca._handleWakeLock);
    }

    jessibuca.$doms.playDom && jessibuca.$doms.playDom.addEventListener('click', (e) => {
        e.stopPropagation();
        jessibuca._play();
    }, false);

    jessibuca.$doms.playBigDom && jessibuca.$doms.playBigDom.addEventListener('click', (e) => {
        e.stopPropagation();
        jessibuca._play();
    }, false);


    jessibuca.$doms.pauseDom && jessibuca.$doms.pauseDom.addEventListener('click', (e) => {
        e.stopPropagation();
        jessibuca._pause();
    }, false);

    // screenshots
    jessibuca.$doms.screenshotsDom && jessibuca.$doms.screenshotsDom.addEventListener('click', (e) => {
        e.stopPropagation();
        const filename = jessibuca._opt.text + '' + now();
        jessibuca._screenshot(filename);
    }, false);


    jessibuca.$doms.fullscreenDom && jessibuca.$doms.fullscreenDom.addEventListener('click', (e) => {
        e.stopPropagation();
        jessibuca.fullscreen = true;
    }, false);

    jessibuca.$doms.minScreenDom && jessibuca.$doms.minScreenDom.addEventListener('click', (e) => {
        e.stopPropagation();
        jessibuca.fullscreen = false;
    }, false);

    jessibuca.$doms.quietAudioDom && jessibuca.$doms.quietAudioDom.addEventListener('click', (e) => {
        e.stopPropagation();
        jessibuca._cancelMute();
    }, false);

    jessibuca.$doms.playAudioDom && jessibuca.$doms.playAudioDom.addEventListener('click', (e) => {
        e.stopPropagation();
        jessibuca._mute();
    }, false);

    //
    jessibuca._enableWakeLock();
}