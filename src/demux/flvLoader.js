import { FLV_MEDIA_TYPE, MEDIA_TYPE } from "../constant";
import CommonLoader from "./commonLoader";
import { now } from "../utils";
import OPut from "oput";

export default class FlvLoader extends CommonLoader {
    constructor(player) {
        super(player);
        this.input = new OPut(this.demux());
        player.debug.log('FlvDemux', 'init');
    }

    destroy() {
        super.destroy();
        this.player.debug.log('FlvDemux', 'destroy');
    }

    dispatch(data) {
        this.input.write(data);
    }

    * demux() {
        yield 9;
        const tmp = new ArrayBuffer(4);
        const tmp8 = new Uint8Array(tmp);
        const tmp32 = new Uint32Array(tmp);
        const player = this.player;
        while (true) {
            tmp8[3] = 0;
            const t = yield 15;
            const type = t[4];
            tmp8[0] = t[7];
            tmp8[1] = t[6];
            tmp8[2] = t[5];
            const length = tmp32[0];
            tmp8[0] = t[10];
            tmp8[1] = t[9];
            tmp8[2] = t[8];
            let ts = tmp32[0];
            if (ts === 0xFFFFFF) {
                tmp8[3] = t[11];
                ts = tmp32[0];
            }
            const payload = (yield length).slice();
            switch (type) {
                case FLV_MEDIA_TYPE.audio:
                    if (player._opt.hasAudio) {
                        player.updateStats({
                            abps: payload.byteLength
                        });
                        if (payload.byteLength > 0) {
                            this._doDecode(payload, MEDIA_TYPE.audio, ts);
                        }
                    }
                    break;
                case FLV_MEDIA_TYPE.video:
                    if (!player._times.demuxStart) {
                        player._times.demuxStart = now();
                    }
                    if (player._opt.hasVideo) {
                        player.updateStats({
                            vbps: payload.byteLength
                        });
                        const isIFrame = payload[0] >> 4 === 1;
                        if (payload.byteLength > 0) {
                            this._doDecode(payload, MEDIA_TYPE.video, ts, isIFrame);
                        }
                    }
                    break;
            }
        }
    }

    close() {
        this.input && this.input.return(null);
    }

}
