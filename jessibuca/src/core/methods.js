import {noop} from "../utils";

export default (jessibuca)=>{
    jessibuca.onPlay = noop;
    jessibuca.onPause = noop;
    jessibuca.onRecord = noop;
    jessibuca.onFullscreen = noop;
    jessibuca.onMute = noop;
    jessibuca.onLoad = noop;
    jessibuca.onLog = noop;
    jessibuca.onError = noop;
    jessibuca.onTimeUpdate = noop;
    jessibuca.onInitSize = noop;
}