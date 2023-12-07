import {FLV_MEDIA_TYPE, MEDIA_TYPE} from "../constant";
import CommonLoader from "./commonLoader";
import {now} from "../utils";

export default class M7sLoader extends CommonLoader {
    constructor(player) {
        super(player);
        player.debug.log('M7sDemux', 'init')
    }

    destroy() {
        super.destroy();
        this.player.debug.log('M7sDemux', 'destroy')
        this.player = null;
    }

    dispatch(data) {
        const player = this.player;
        const dv = new DataView(data)
        const type = dv.getUint8(0);
        const ts = dv.getUint32(1, false);
        const tmp = new ArrayBuffer(4);
        const tmp32 = new Uint32Array(tmp);
        switch (type) {
            case MEDIA_TYPE.audio:
                if (player._opt.hasAudio) {
                    const payload = new Uint8Array(data, 5)
                    player.updateStats({
                        abps: payload.byteLength
                    })
                    if (payload.byteLength > 0) {
                        this._doDecode(payload, type, ts)
                    }
                }
                break;
            case MEDIA_TYPE.video:
                if (player._opt.hasVideo) {
                    if (!player._times.demuxStart) {
                        player._times.demuxStart = now();
                    }
                    if (dv.byteLength > 5) {
                        const payload = new Uint8Array(data, 5);

                        const flags = payload[0];
                        if (this._isEnhancedH265Header(flags)) {
                            this._decodeEnhancedH265Video(payload, ts);
                        } else {
                            const isIframe = dv.getUint8(5) >> 4 === 1;
                            player.updateStats({
                                vbps: payload.byteLength
                            })

                            tmp32[0] = payload[4]
                            tmp32[1] = payload[3]
                            tmp32[2] = payload[2]
                            tmp32[3] = 0
                            let cts = tmp32[0]
                            this._doDecode(payload, type, ts, isIframe, cts);
                        }
                    } else {
                        this.player.debug.warn('M7sDemux', 'dispatch', 'dv byteLength is', dv.byteLength)
                    }
                }
                break;
        }
    }


}
