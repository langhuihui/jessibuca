import {FLV_MEDIA_TYPE, MEDIA_TYPE} from "../constant";
import CommonLoader from "./commonLoader";
import {now} from "../utils";

export default class FlvLoader extends CommonLoader {
    constructor(player) {
        super(player);
        this.input = this._inputFlv();
        this.flvDemux = this.dispatchFlvData(this.input);
        player.debug.log('FlvDemux', 'init')
    }

    destroy() {
        super.destroy();
        this.input = null;
        this.flvDemux = null;
        this.player.debug.log('FlvDemux', 'destroy')
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
                        if (payload.byteLength > 0) {
                            this._doDecode(payload, MEDIA_TYPE.audio, ts)
                        }
                    }
                    break
                case FLV_MEDIA_TYPE.video:
                    if (!player._times.demuxStart) {
                        player._times.demuxStart = now();
                    }
                    if (player._opt.hasVideo) {
                        player.updateStats({
                            vbps: payload.byteLength
                        })
                        const isIFrame = payload[0] >> 4 === 1;
                        if (payload.byteLength > 0) {
                            this._doDecode(payload, MEDIA_TYPE.video, ts, isIFrame);
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
        this.input && this.input.return(null)
    }


}
