import {now} from "../utils";

export default (jessibuca) => {
    jessibuca._resize$2 = () => jessibuca.resize();

    jessibuca._handleVisibilityChange$2 = () => jessibuca._handleVisibilityChange();
    jessibuca._onfullscreenchange$2 = () => jessibuca._onfullscreenchange();
    jessibuca._handleWakeLock$2 = () => jessibuca._handleWakeLock();

    window.addEventListener('resize', jessibuca._resize$2);
    window.addEventListener('fullscreenchange', jessibuca._onfullscreenchange$2);
    document.addEventListener('visibilitychange', jessibuca._handleVisibilityChange$2);
    document.addEventListener('visibilitychange', jessibuca._handleWakeLock$2);
    window.addEventListener('fullscreenchange', jessibuca._handleWakeLock$2);


    if (jessibuca._opt.supportDblclickFullscreen) {
        jessibuca.$canvasElement.addEventListener('dblclick', () => {
            jessibuca.fullscreen = !jessibuca.fullscreen;
        }, false);
    }

    jessibuca._removeEventListener = () => {
        window.removeEventListener('resize', jessibuca._resize$2);
        window.removeEventListener('fullscreenchange', jessibuca._onfullscreenchange$2);
        document.removeEventListener('visibilitychange', jessibuca._handleWakeLock$2);
        document.removeEventListener('visibilitychange', jessibuca._handleVisibilityChange$2);
        window.removeEventListener('fullscreenchange', jessibuca._handleWakeLock$2);
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

    jessibuca.$doms.recordDom && jessibuca.$doms.recordDom.addEventListener('click', (e) => {
        e.stopPropagation();
        jessibuca.recording = true;
    }, false);
    //
    jessibuca.$doms.recordingDom && jessibuca.$doms.recordingDom.addEventListener('click', (e) => {
        e.stopPropagation();
        jessibuca.recording = false;
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