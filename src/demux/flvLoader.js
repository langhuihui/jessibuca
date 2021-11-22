import {FLV_MEDIA_TYPE} from "../constant";

export default class FlvLoader {
    constructor(player) {
        this.player = player;
        const input = this._inputFlv();
        this.flvDemux = this.dispatchFlvData(input);
        player.debug.log('FlvDemux', 'init')
    }

    dispatch(data) {
        this.flvDemux(data);
    }

    * _inputFlv() {
        yield 9;
        const tmp = new ArrayBuffer(4)
        const tmp8 = new Uint8Array(tmp)
        const tmp32 = new Uint32Array(tmp)

        const player = this.player;
        const {decoderWorker, webcodecsDecoder} = player;

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
                        decoderWorker.decodeAudio(payload, ts);
                    }
                    break
                case FLV_MEDIA_TYPE.video:
                    if (player._opt.hasVideo) {
                        const isIframe = payload[0] >> 4 === 1;
                        if (player._opt.useWCS) {
                            webcodecsDecoder.decodeVideo(payload, ts, isIframe);
                        } else {
                            decoderWorker.decodeVideo(payload, ts, isIframe);
                        }
                    }
                    break
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

    }
}
