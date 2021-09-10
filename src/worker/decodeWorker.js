import { CMD_TYPE, EVEMTS, POST_MESSAGE } from "../constant";
import { bpsSize } from "../utils";

export default (jessibuca) => {
    const decoderWorker = new Worker(jessibuca._opt.decoder);
    decoderWorker.onmessage = (event) => {
        const msg = event.data;
        switch (msg.cmd) {
            case CMD_TYPE.init:
                jessibuca.setBufferTime(jessibuca._opt.videoBuffer);
                decoderWorker.postMessage({
                    cmd: POST_MESSAGE.init,
                    opt: JSON.stringify(jessibuca._opt),
                    sampleRate: jessibuca._audioContext.sampleRate
                })
                if (!jessibuca._hasLoaded) {
                    jessibuca._hasLoaded = true;
                    jessibuca.onLoad();
                    jessibuca._trigger(EVEMTS.load);
                }
                break;
            case CMD_TYPE.initSize:
                jessibuca.$canvasElement.width = msg.w;
                jessibuca.$canvasElement.height = msg.h;
                jessibuca.onInitSize();
                jessibuca._resize();
                jessibuca._trigger(EVEMTS.videoInfo, { w: msg.w, h: msg.h });
                jessibuca._trigger(EVEMTS.start);
                if (jessibuca._supportOffscreen()) {
                    jessibuca._bitmaprenderer = jessibuca.$canvasElement.getContext("bitmaprenderer");

                }
                break;
            case CMD_TYPE.render:
                if (jessibuca.loading) {
                    jessibuca.loading = false;
                    jessibuca.playing = true;
                    jessibuca._clearCheckLoading();
                }
                if (jessibuca.playing) {
                    if (!jessibuca._supportOffscreen()) {
                        jessibuca._contextGLRender(jessibuca.$canvasElement.width, jessibuca.$canvasElement.height, msg.output[0], msg.output[1], msg.output[2])
                    } else {
                        jessibuca._bitmaprenderer.transferFromImageBitmap(msg.buffer);
                    }
                }
                jessibuca._trigger(EVEMTS.timeUpdate, msg.ts);
                jessibuca.onTimeUpdate(msg.ts);
                jessibuca._updateStats({ buf: msg.delay, ts: msg.ts });
                jessibuca._checkHeart();
                break;
            case CMD_TYPE.playAudio:
                if (jessibuca.playing && !jessibuca.quieting) {
                    jessibuca._playAudio(msg.buffer)
                }
                break
            case CMD_TYPE.print:
                jessibuca.onLog(msg.text);
                jessibuca._trigger(EVEMTS.log, msg.text);
                break;
            case CMD_TYPE.printErr:
                jessibuca.onLog(msg.text);
                jessibuca._trigger(EVEMTS.log, msg.text);
                jessibuca.onError(msg.text);
                jessibuca._trigger(EVEMTS.error, msg.text);
                break;
            case CMD_TYPE.initAudioPlanar:
                jessibuca._initAudioPlanar(msg);
                jessibuca._trigger(EVEMTS.audioInfo, {
                    numOfChannels: msg.channels, // 声频通道
                    sampleRate: msg.samplerate // 采样率
                });
                if(jessibuca._opt.audioOnly){

                    if (jessibuca.loading) {
                        jessibuca.loading = false;
                        jessibuca.playing = true;
                        jessibuca._clearCheckLoading();
                    }
                    jessibuca._trigger(EVEMTS.start);
                    jessibuca._trigger(EVEMTS.play);
                }
                break;
            case CMD_TYPE.kBps:
                if (jessibuca.playing) {
                    jessibuca.$doms.speedDom && (jessibuca.$doms.speedDom.innerText = bpsSize(msg.kBps));
                    jessibuca._trigger(EVEMTS.kBps, msg.kBps);
                }
            default:
                jessibuca[msg.cmd] && jessibuca[msg.cmd](msg);
        }
    }

    jessibuca._decoderWorker = decoderWorker;
}
