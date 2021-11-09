import {EVENTS, MEDIA_TYPE, WORKER_CMD_TYPE, WORKER_SEND_TYPE} from "../constant";

export default class DecoderWorker {
    constructor(player) {
        this.player = player;
        this.decoderWorker = new Worker(player._opt.decoder)
        const {
            debug,
            events: {proxy},
        } = player;

        this.decoderWorker.onmessage = (event) => {
            const msg = event.data;
            switch (msg.cmd) {
                case WORKER_CMD_TYPE.init:
                    debug.log(`decoderWorker`, 'onmessage:', WORKER_CMD_TYPE.init);
                    this.player.emit(EVEMTS.load);
                    this.player.emit(EVEMTS.decoderWorkerInit);
                    this._initWork();
                    break;
                case WORKER_CMD_TYPE.initVideo:
                    debug.log(`decoderWorker`, 'onmessage:', WORKER_CMD_TYPE.initVideo, `width:${msg.w},height:${msg.h}`);
                    if (this.playType === PLAY_TYPE.player) {
                        this.player.video.bindOffscreen();
                        this.player.emit(EVEMTS.start);
                    } else if (this.playType === PLAY_TYPE.playbackTF) {
                        this.player.video.initVideoDelay();
                    } else if (this.playType === PLAY_TYPE.playbackCloud) {
                        this.player.video.initVideo();
                    }
                    break;
                case WORKER_CMD_TYPE.initAudio:
                    debug.log(`decoderWorker`, 'onmessage:', WORKER_CMD_TYPE.initAudio, `channels:${msg.channels},samplerate:${msg.samplerate}`);
                    if (this.playType === PLAY_TYPE.player) {
                        this.player.audio.initScriptNode(msg);
                    } else if (this.playType === PLAY_TYPE.playbackTF) {
                        this.player.audio.initScriptNodeDelay(msg);
                    } else if (this.playType === PLAY_TYPE.playbackCloud) {
                        this.player.audio.initScriptNode(msg);
                    }
                    break;
                case WORKER_CMD_TYPE.render:
                    // debug.log(`decoderWorker`, 'onmessage:', WORKER_CMD_TYPE.render, `msg ts:${msg.ts}`);
                    if (this.player.loading) {
                        this.player.emit(EVEMTS.frameStart);
                        this.player.loading = false;
                    }
                    if (!this.player.playing) {
                        this.player.playing = true;
                    }
                    if (this.playType === PLAY_TYPE.player) {
                        this.player.video.render(msg);
                    } else if (this.playType === PLAY_TYPE.playbackTF) {
                        this.player.video.pushData(msg);
                    } else if (this.playType === PLAY_TYPE.playbackCloud) {
                        this.player.video.pushData(msg);
                    }
                    break;

                case WORKER_CMD_TYPE.playAudio:
                    // debug.log(`decoderWorker`, 'onmessage:', WORKER_CMD_TYPE.playAudio, `msg ts:${msg.ts}`);
                    if (this.playType === PLAY_TYPE.player) {
                        // 只有在 playing 的时候。
                        if (this.player.playing) {
                            this.player.audio.play(msg.buffer, msg.ts);
                        }
                    } else if (this.playType === PLAY_TYPE.playbackTF) {
                        // 只有在 playing 的时候。
                        if (this.player.playing) {
                            this.player.audio.play(msg.buffer, msg.ts);
                        }
                    } else if (this.playType === PLAY_TYPE.playbackCloud) {
                        //
                        if (this.player.playing) {
                            this.player.audio.play(msg.buffer, msg.ts);
                        }
                    }
                    break;
                default:
                    player[msg.cmd] && player[msg.cmd](msg);
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

        if (this.player._opt.useWCS && this.player._opt.forceNoOffscreen) {

        } else {
            this.decoderWorker.postMessage({
                cmd: WORKER_SEND_TYPE.decode,
                buffer: arrayBuffer,
                options
            }, [arrayBuffer.buffer])
        }
    }

    //
    decodeAudio(arrayBuffer, ts) {
        const options = {
            type: MEDIA_TYPE.audio,
            ts: Math.max(ts, 0)
        }
        this.decoderWorker.postMessage({
            cmd: WORKER_SEND_TYPE.decode,
            buffer: arrayBuffer,
            options
        }, [arrayBuffer.buffer])
    }


    destroy() {
        this.player.debug.log(`decoderWorker`, 'destroy');
        this.decoderWorker.postMessage({cmd: WORKER_SEND_TYPE.close})
        this.decoderWorker.terminate();
        this.decoderWorker = null;
        this.player = null;
    }
}
