import {EVENTS, MEDIA_TYPE, WORKER_CMD_TYPE, WORKER_SEND_TYPE} from "../constant";

export default class DecoderWorker {
    constructor(player) {
        this.player = player;
        this.decoderWorker = new Worker(player._opt.decoder)
        this._initDecoderWorker();
        player.debug.log('decoderWorker', 'init')
    }

    _initDecoderWorker() {
        const {
            debug,
            events: {proxy},
        } = this.player;

        this.decoderWorker.onmessage = (event) => {
            const msg = event.data;
            switch (msg.cmd) {
                case WORKER_CMD_TYPE.init:
                    debug.log(`decoderWorker`, 'onmessage:', WORKER_CMD_TYPE.init);
                    if (!this.player.loaded) {
                        this.player.emit(EVENTS.load);
                    }
                    this.player.emit(EVENTS.decoderWorkerInit);
                    this._initWork();
                    break;
                case WORKER_CMD_TYPE.videoCode:
                    debug.log(`decoderWorker`, 'onmessage:', WORKER_CMD_TYPE.videoCode, msg.code);
                    this.player.video.updateVideoInfo({
                        encTypeCode: msg.code
                    })
                    break;
                case WORKER_CMD_TYPE.audioCode:
                    debug.log(`decoderWorker`, 'onmessage:', WORKER_CMD_TYPE.audioCode, msg.code);
                    this.player.audio.updateAudioInfo({
                        encTypeCode: msg.code
                    })
                    break;
                case WORKER_CMD_TYPE.initVideo:
                    debug.log(`decoderWorker`, 'onmessage:', WORKER_CMD_TYPE.initVideo, `width:${msg.w},height:${msg.h}`);
                    this.player.video.updateVideoInfo({
                        width: msg.w,
                        height: msg.h
                    })
                    this.player.video.initCanvasViewSize();
                    break;
                case WORKER_CMD_TYPE.initAudio:
                    debug.log(`decoderWorker`, 'onmessage:', WORKER_CMD_TYPE.initAudio, `channels:${msg.channels},sampleRate:${msg.sampleRate}`);
                    this.player.audio.updateAudioInfo(msg);
                    this.player.audio.initScriptNode(msg);
                    break;
                case WORKER_CMD_TYPE.render:
                    // debug.log(`decoderWorker`, 'onmessage:', WORKER_CMD_TYPE.render, `msg ts:${msg.ts}`);
                    this.player.handleRender();
                    this.player.video.render(msg);
                    this.player.emit(EVENTS.timeUpdate, msg.ts)
                    this.player.updateStats({fps: true, ts: msg.ts, buf: msg.delay})
                    break;
                case WORKER_CMD_TYPE.playAudio:
                    // debug.log(`decoderWorker`, 'onmessage:', WORKER_CMD_TYPE.playAudio, `msg ts:${msg.ts}`);
                    // 只有在 playing 的时候。
                    if (this.player.playing) {
                        this.player.audio.play(msg.buffer, msg.ts);
                    }
                    break;
                default:
                    this.player[msg.cmd] && this.player[msg.cmd](msg);
            }
        }
    }

    _initWork() {
        this.decoderWorker.postMessage({
            cmd: WORKER_SEND_TYPE.init,
            opt: JSON.stringify(this.player._opt),
            sampleRate: this.player.audio.audioContext.sampleRate
        })
    }

    decodeVideo(arrayBuffer, ts, isIFrame) {
        const options = {
            type: MEDIA_TYPE.video,
            ts: Math.max(ts, 0),
            isIFrame
        }
        // this.player.debug.log('decoderWorker', 'decodeVideo', options);
        this.decoderWorker.postMessage({
            cmd: WORKER_SEND_TYPE.decode,
            buffer: arrayBuffer,
            options
        }, [arrayBuffer.buffer])
    }

    decodeAudio(arrayBuffer, ts) {
        if (this.player._opt.useWCS && !this.player._opt.useOffscreen) {
            this._decodeAudioNoDelay(arrayBuffer, ts);
        } else if (this.player._opt.useMSE) {
            this._decodeAudioNoDelay(arrayBuffer, ts);
        } else {
            this._decodeAudio(arrayBuffer, ts);
        }
    }

    //
    _decodeAudio(arrayBuffer, ts) {
        const options = {
            type: MEDIA_TYPE.audio,
            ts: Math.max(ts, 0)
        }
        // this.player.debug.log('decoderWorker', 'decodeAudio',options);
        this.decoderWorker.postMessage({
            cmd: WORKER_SEND_TYPE.decode,
            buffer: arrayBuffer,
            options
        }, [arrayBuffer.buffer])
    }


    _decodeAudioNoDelay(arrayBuffer, ts) {
        // console.log('_decodeAudioNoDelay', arrayBuffer);
        this.decoderWorker.postMessage({
            cmd: WORKER_SEND_TYPE.audioDecode,
            buffer: arrayBuffer,
            ts: Math.max(ts, 0)
        }, [arrayBuffer.buffer])
    }

    destroy() {
        this.decoderWorker.postMessage({cmd: WORKER_SEND_TYPE.close})
        this.decoderWorker.terminate();
        this.decoderWorker = null;
        this.player.debug.log(`decoderWorker`, 'destroy');
    }
}
