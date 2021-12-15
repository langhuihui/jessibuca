import {FLV_MEDIA_TYPE, MEDIA_TYPE} from "../constant";
import CommonLoader from "./commonLoader";

export default class M7sLoader extends CommonLoader {
    constructor(player) {
        super(player);
        player.debug.log('M7sDemux', 'init')
    }

    dispatch(data) {
        const player = this.player;
        const {decoderWorker, webcodecsDecoder, mseDecoder} = player;
        const dv = new DataView(data)
        const type = dv.getUint8(0);
        const ts = dv.getUint32(1, false);
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
                    if (dv.byteLength > 5) {
                        const payload = new Uint8Array(data, 5);
                        const isIframe = dv.getUint8(5) >> 4 === 1;
                        player.updateStats({
                            vbps: payload.byteLength
                        })
                        if (payload.byteLength > 0) {
                            this._doDecode(payload, type, ts, isIframe)
                        }
                    }
                }
                break;
        }
    }

    destroy() {
        super.destroy();
        this.player.debug.log('M7sDemux', 'destroy')
    }
}
