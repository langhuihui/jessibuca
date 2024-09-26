import Emitter from "../utils/emitter";
import {FRAME_HEADER_EX, FRAME_TYPE_EX, MEDIA_TYPE, PACKET_TYPE_EX} from "../constant";
import {hevcEncoderNalePacketNotLength} from "../utils";

export default class CommonLoader extends Emitter {
    constructor(player) {
        super();
        this.player = player;

        this.stopId = null;
        this.firstTimestamp = null;
        this.startTimestamp = null;
        this.delay = -1;
        this.bufferList = [];
        this.dropping = false;
        this.initInterval();
    }

    destroy() {
        if (this.stopId) {
            clearInterval(this.stopId);
            this.stopId = null;
        }
        this.firstTimestamp = null;
        this.startTimestamp = null;
        this.delay = -1;
        this.bufferList = [];
        this.dropping = false;
        this.off();
        this.player.debug.log('CommonDemux', 'destroy');
    }

    getDelay(timestamp) {
        if (!timestamp) {
            return -1
        }
        if (!this.firstTimestamp) {
            this.firstTimestamp = timestamp
            this.startTimestamp = Date.now()
            this.delay = -1;
        } else {
            if (timestamp) {
                const localTimestamp = (Date.now() - this.startTimestamp);
                const timeTimestamp = (timestamp - this.firstTimestamp);
                if (localTimestamp >= timeTimestamp) {
                    this.delay = localTimestamp - timeTimestamp;
                } else {
                    this.delay = timeTimestamp - localTimestamp;
                }
            }
        }
        return this.delay
    }

    resetDelay() {
        this.firstTimestamp = null;
        this.startTimestamp = null;
        this.delay = -1;
        this.dropping = false;
    }

    //
    initInterval() {
        this.player.debug.log('common dumex', `init Interval`);
        let _loop = () => {
            let data;
            const videoBuffer = this.player._opt.videoBuffer;
            const videoBufferDelay = this.player._opt.videoBufferDelay;
            if (this.player.isDestroyedOrClosed()) {
                return;
            }

            if (this.player._opt.useMSE &&
                this.player.mseDecoder &&
                this.player.mseDecoder.getSourceBufferUpdating()) {
                this.player.debug.warn('CommonDemux', `_loop getSourceBufferUpdating is true and bufferList length is ${this.bufferList.length}`);
                return;
            }

            if (this.bufferList.length) {
                if (this.dropping) {
                    // this.player.debug.log('common dumex', `is dropping`);
                    data = this.bufferList.shift();
                    if (data.type === MEDIA_TYPE.audio && data.payload[1] === 0) {
                        this._doDecoderDecode(data);
                    }
                    while (!data.isIFrame && this.bufferList.length) {
                        data = this.bufferList.shift();
                        if (data.type === MEDIA_TYPE.audio && data.payload[1] === 0) {
                            this._doDecoderDecode(data);
                        }
                    }
                    // i frame
                    if (data.isIFrame && this.getDelay(data.ts) <= Math.min(videoBuffer, 200)) {
                        this.dropping = false;
                        this._doDecoderDecode(data);
                    }
                } else {
                    data = this.bufferList[0];
                    if (this.getDelay(data.ts) === -1) {
                        // this.player.debug.log('common dumex', `delay is -1`);
                        this.bufferList.shift()
                        this._doDecoderDecode(data);
                    } else if (this.delay > (videoBuffer + videoBufferDelay)) {
                        // this.player.debug.log('common dumex', `delay is ${this.delay}, set dropping is true`);
                        this.resetDelay();
                        this.dropping = true
                    } else {
                        data = this.bufferList[0]
                        if (this.getDelay(data.ts) > videoBuffer) {
                            // drop frame
                            this.bufferList.shift()
                            this._doDecoderDecode(data);
                        } else {
                            // this.player.debug.log('common dumex', `delay is ${this.delay}`);
                        }
                    }
                }
            }
        }
        _loop();
        this.stopId = setInterval(_loop, 10)
    }

    _doDecode(payload, type, ts, isIFrame, cts) {
        const player = this.player;
        let options = {
            ts: ts,
            cts: cts,
            type: type,
            isIFrame: false
        }
        // use offscreen
        if (player._opt.useWCS && !player._opt.useOffscreen) {
            if (type === MEDIA_TYPE.video) {
                options.isIFrame = isIFrame;
            }
            this.pushBuffer(payload, options)
        } else if (player._opt.useMSE) {
            // use mse
            if (type === MEDIA_TYPE.video) {
                options.isIFrame = isIFrame;
            }
            this.pushBuffer(payload, options)
        } else {
            //
            if (type === MEDIA_TYPE.video) {
                player.decoderWorker && player.decoderWorker.decodeVideo(payload, ts, isIFrame);
            } else if (type === MEDIA_TYPE.audio) {
                if (player._opt.hasAudio) {
                    player.decoderWorker && player.decoderWorker.decodeAudio(payload, ts);
                }
            }
        }
    }

    _doDecoderDecode(data) {
        const player = this.player;
        const {webcodecsDecoder, mseDecoder} = player;

        if (data.type === MEDIA_TYPE.audio) {
            if (player._opt.hasAudio) {
                player.decoderWorker && player.decoderWorker.decodeAudio(data.payload, data.ts)
            }
        } else if (data.type === MEDIA_TYPE.video) {
            if (player._opt.useWCS && !player._opt.useOffscreen) {
                webcodecsDecoder.decodeVideo(data.payload, data.ts, data.isIFrame);
            } else if (player._opt.useMSE) {
                mseDecoder.decodeVideo(data.payload, data.ts, data.isIFrame, data.cts);
            }
        }
    }

    pushBuffer(payload, options) {
        // 音频
        if (options.type === MEDIA_TYPE.audio) {
            this.bufferList.push({
                ts: options.ts,
                payload: payload,
                type: MEDIA_TYPE.audio,
            })
        } else if (options.type === MEDIA_TYPE.video) {
            this.bufferList.push({
                ts: options.ts,
                cts: options.cts,
                payload: payload,
                type: MEDIA_TYPE.video,
                isIFrame: options.isIFrame
            })
        }
    }

    close() {

    }

    _decodeEnhancedH265Video(payload, ts) {
        const flags = payload[0];
        const frameTypeEx = flags & 0x30;
        const packetEx = flags & 0x0F;
        const codecId = payload.slice(1, 5);
        const tmp = new ArrayBuffer(4);
        const tmp32 = new Uint32Array(tmp);
        const isAV1 = String.fromCharCode(codecId[0]) == 'a';
        if (packetEx === PACKET_TYPE_EX.PACKET_TYPE_SEQ_START) {
            if (frameTypeEx === FRAME_TYPE_EX.FT_KEY) {
                // header video info
                const extraData = payload.slice(5);
                if (!isAV1) {
                    const payloadBuffer = new Uint8Array(5 + extraData.length);
                    payloadBuffer.set([0x1c, 0x00, 0x00, 0x00, 0x00], 0);
                    payloadBuffer.set(extraData, 5);
                    this._doDecode(payloadBuffer, MEDIA_TYPE.video, 0, true, 0);
                }
            }
        } else if (packetEx === PACKET_TYPE_EX.PACKET_TYPE_FRAMES) {
            let payloadBuffer = payload;
            let cts = 0;
            const isIFrame = frameTypeEx === FRAME_TYPE_EX.FT_KEY;

            if (!isAV1) {
                // h265
                tmp32[0] = payload[4]
                tmp32[1] = payload[3]
                tmp32[2] = payload[2]
                tmp32[3] = 0
                cts = tmp32[0];
                const data = payload.slice(8);
                payloadBuffer = hevcEncoderNalePacketNotLength(data, isIFrame);
                this._doDecode(payloadBuffer, MEDIA_TYPE.video, ts, isIFrame, cts);
            }

        } else if (packetEx === PACKET_TYPE_EX.PACKET_TYPE_FRAMESX) {
            const isIFrame = frameTypeEx === FRAME_TYPE_EX.FT_KEY;
            const data = payload.slice(5);
            let payloadBuffer = hevcEncoderNalePacketNotLength(data, isIFrame);
            this._doDecode(payloadBuffer, MEDIA_TYPE.video, ts, isIFrame, 0);
        }
    }


    _isEnhancedH265Header(flags) {
        return (flags & FRAME_HEADER_EX) === FRAME_HEADER_EX;
    }
}
