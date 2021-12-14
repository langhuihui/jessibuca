import {FLV_MEDIA_TYPE, MEDIA_TYPE} from "../constant";
import CommonLoader from "./commonLoader";

export default class FlvLoader extends CommonLoader {
    constructor(player) {
        super(player);
        // this.player = player;
        const input = this._inputFlv();
        this.flvDemux = this.dispatchFlvData(input);
        player.debug.log('FlvDemux', 'init')
    }

    dispatch(data) {
        // this.player.debug.log('FlvDemux', 'dispatch');
        this.flvDemux(data);
    }

    * _inputFlv() {
        yield 9;
        const tmp = new ArrayBuffer(4)
        const tmp8 = new Uint8Array(tmp)
        const tmp32 = new Uint32Array(tmp)

        const player = this.player;
        const {decoderWorker, webcodecsDecoder, mseDecoder} = player;

        while (true) {
            tmp8[3] = 0
            const t = yield 15
            const type = t[4]
            tmp8[0] = t[7]
            tmp8[1] = t[6]
            tmp8[2] = t[5]
            const length = tmp32[0]
            tmp8[0] = t[10]
            tmp8[1] = t[9]
            tmp8[2] = t[8]
            let ts = tmp32[0]
            if (ts === 0xFFFFFF) {
                tmp8[3] = t[11]
                ts = tmp32[0]
            }
            const payload = yield length
            switch (type) {
                case FLV_MEDIA_TYPE.audio:
                    if (player._opt.hasAudio) {
                        player.updateStats({
                            abps: payload.byteLength
                        })
                        decoderWorker.decodeAudio(payload, ts)
                        // this._doDecode(payload, type, ts)
                    }
                    break
                case FLV_MEDIA_TYPE.video:
                    if (player._opt.hasVideo) {
                        player.updateStats({
                            vbps: payload.byteLength
                        })
                        const isIframe = payload[0] >> 4 === 1;
                        // 没有使用离屏渲染
                        if (player._opt.useWCS && !player._opt.useOffscreen) {
                            // this.player.debug.log('FlvDemux', 'decodeVideo useWCS')
                            webcodecsDecoder.decodeVideo(payload, ts, isIframe);
                        } else if (player._opt.useMSE) {
                            // this.player.debug.log('FlvDemux', 'decodeVideo useMSE')
                            mseDecoder.decodeVideo(payload, ts, isIframe);
                        } else {
                            // this.player.debug.log('FlvDemux', 'decodeVideo')
                            decoderWorker.decodeVideo(payload, ts, isIframe);
                        }
                        // this._doDecode(payload, type, ts, isIframe);

                    }
                    break
            }
        }
    }

    _doDecode(payload, type, ts, isIframe) {
        const player = this.player;
        const {decoderWorker, webcodecsDecoder, mseDecoder} = player;
        let options = {
            ts: ts,
            type: null,
        }
        if (player._opt.useWCS && !player._opt.useOffscreen) {
            if (type === FLV_MEDIA_TYPE.video) {
                options.type = MEDIA_TYPE.video;
            } else if (type === FLV_MEDIA_TYPE.audio) {
                options.type = MEDIA_TYPE.audio;
            }
            // this.pushBuffer(payload, options)
            webcodecsDecoder.decodeVideo(payload, ts, isIframe);
        } else if (player._opt.useMSE) {
            if (type === FLV_MEDIA_TYPE.video) {
                options.type = MEDIA_TYPE.video;
            } else if (type === FLV_MEDIA_TYPE.audio) {
                options.type = MEDIA_TYPE.audio;
            }

            // this.pushBuffer(payload, options)
            mseDecoder.decodeVideo(payload, ts, isIframe);
        } else {
            if (type === FLV_MEDIA_TYPE.video) {
                decoderWorker.decodeVideo(payload, ts, isIframe);
            } else if (type === FLV_MEDIA_TYPE.audio) {
                decoderWorker.decodeAudio(payload, ts);
            }
        }
    }

    dispatchFlvData(input) {
        let need = input.next()
        let buffer = null
        return (value) => {
            let data = new Uint8Array(value)
            if (buffer) {
                let combine = new Uint8Array(buffer.length + data.length)
                combine.set(buffer)
                combine.set(data, buffer.length)
                data = combine
                buffer = null
            }
            while (data.length >= need.value) {
                let remain = data.slice(need.value)
                need = input.next(data.slice(0, need.value))
                data = remain
            }
            if (data.length > 0) {
                buffer = data
            }
        }
    }

    close() {

    }

    destroy() {
        super.destroy();
        this.player.debug.log('FlvDemux', 'destroy')
    }
}
