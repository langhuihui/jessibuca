import {$domToggle} from "../utils";
import {EVEMTS} from "../constant";

export default (jessibuca) => {

    jessibuca._loading = true;
    jessibuca._recording = false;
    jessibuca._playing = false;
    jessibuca._audioPlaying = false;
    jessibuca._quieting = true;
    jessibuca._fullscreen = false;

    jessibuca._stats = {
        buf: 0, //ms
        fps: 0,
        abps: '',
        vbps: '',
        ts: ''
    }

    jessibuca._hasLoaded = false;
    jessibuca._playUrl = '';
    jessibuca._startBpsTime = '';
    jessibuca._bps = 0;
    jessibuca._checkHeartTimeout = null;
    jessibuca._wakeLock = null;
    jessibuca._contextGL = null;
    jessibuca._contextGLRender = null;
    jessibuca._checkLoadingTimeout = null;
    jessibuca._bitmaprenderer = null;


    jessibuca._initCheckVariable = () => {
        jessibuca._startBpsTime = '';
        jessibuca._bps = 0;
        jessibuca._clearCheckHeartTimeout();
    }

    jessibuca._clearCheckHeartTimeout = () => {
        if (jessibuca._checkHeartTimeout) {
            clearTimeout(jessibuca._checkHeartTimeout);
            jessibuca._checkHeartTimeout = null;
        }
    }

    jessibuca._startCheckHeartTimeout = () => {
        jessibuca._checkHeartTimeout = setTimeout(function () {
            jessibuca._trigger(EVEMTS.timeout);
            jessibuca.recording = false;
            jessibuca.playing = false;
            jessibuca._close();
        }, jessibuca._opt.timeout * 1000);
    }


    jessibuca._clearCheckLoading = () => {
        if (jessibuca._checkLoadingTimeout) {
            clearTimeout(jessibuca._checkLoadingTimeout);
            jessibuca._checkLoadingTimeout = null;
        }
    }

    jessibuca._checkLoading = () => {
        jessibuca._clearCheckLoading();
        jessibuca._checkLoadingTimeout = setTimeout(() => {
            jessibuca._trigger(EVEMTS.timeout);
            jessibuca.playing = false;
            jessibuca._close();
            $domToggle(jessibuca.$doms.loadingDom, false);
        }, jessibuca._opt.timeout * 1000);
    }

}