import Emitter from "../utils/emitter";
import {MEDIA_TYPE} from "../constant";

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
                this.delay = (Date.now() - this.startTimestamp) - (timestamp - this.firstTimestamp)
            }
        }
        return this.delay
    }

    //
    initInterval() {
        const videoBuffer = this.player._opt.videoBuffer;
        this.stopId = setInterval(() => {
            let data;
            if (this.bufferList.length) {
                if (this.dropping) {
                    // // 连续的把视频帧给丢掉一直丢到关键帧停下来。。。
                    data = this.bufferList.shift();
                    while (!data.isIFrame && this.bufferList.length) {
                        // 持续丢帧。
                        data = this.bufferList.shift();
                    }

                    if (data.isIFrame) {
                        this.dropping = false;
                        data.decoder.decode(data.payload, data.ts)
                    }
                } else {
                    data = this.bufferList[0];
                    if (this.getDelay(data.ts) === -1) {
                        this.bufferList.shift()
                        data.decoder.decode(data.payload, data.ts)
                    } else if (this.delay > videoBuffer + 1000) {

                    }
                }
            }
        }, 10)
    }

    pushBuffer(payload, options) {
        // 音频
        if (options.type === MEDIA_TYPE.audio) {
            this.bufferList.push({
                ts: options.ts,
                payload: payload,
                decoder: options.decoder,
                type: MEDIA_TYPE.audio,
            })
        } else if (options.type === MEDIA_TYPE.video) {
            this.bufferList.push({
                ts: options.ts,
                payload: payload,
                decoder: options.decoder,
                type: MEDIA_TYPE.video,
                isIFrame: options.isIFrame
            })
        }
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
    }
}
