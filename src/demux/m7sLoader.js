import {MEDIA_TYPE} from "../constant";

export default class M7sLoader {
    constructor(player) {
        this.player = player;
        player.debug.log('M7sDemux', 'init')
    }

    dispatch(data) {
        const player = this.player;
        const {decoderWorker, webcodecsDecoder, mseDecoder} = player;
        const dv = new DataView(data.buffer)
        const type = dv.getUint8(0);
        const ts = dv.getUint32(1, false);
        switch (type) {
            case MEDIA_TYPE.audio:
                if (player._opt.hasAudio) {
                    const payload = new Uint8Array(data, 5)
                    decoderWorker.decodeAudio(payload, ts)
                }
                break;
            case MEDIA_TYPE.video:
                if (player._opt.hasVideo) {
                    if (dv.byteLength > 5) {
                        const payload = new Uint8Array(data, 5);
                        const isIframe = dv.getUint8(5) >> 4 === 1;
                        if (player._opt.useWCS) {
                            // this.player.debug.log('FlvDemux', 'decodeVideo useWCS')
                            webcodecsDecoder.decodeVideo(payload, ts, isIframe);
                        } else if (player._opt.useMSE) {
                            // this.player.debug.log('FlvDemux', 'decodeVideo useMSE')
                            mseDecoder.decodeVideo(payload, ts, isIframe);
                        } else {
                            // this.player.debug.log('FlvDemux', 'decodeVideo')
                            decoderWorker.decodeVideo(payload, ts, isIframe);
                        }
                    }
                }
                break;
        }
    }

    close() {

    }

    destroy() {

    }
}
