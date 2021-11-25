import Emitter from "../utils/emitter";
import {EVENTS, MEDIA_SOURCE_STATE, MP4_CODECS} from "../constant";
import {formatVideoDecoderConfigure} from "../utils";
import MP4 from "../remux/mp4-generator";

export default class MseDecoder extends Emitter {
    constructor(player) {
        super();
        this.player = player;
        this.isAvc = true;
        this.mediaSource = new window.MediaSource;
        this.sourceBuffer = null;
        this.hasInit = false;
        this.isInitInfo = false;
        this.cacheTrack = {};
        this.timeInit = false;

        player.video.$videoElement.src = window.URL.createObjectURL(this.mediaSource);

        const {
            debug,
            events: {proxy},
        } = player;


        proxy(this.mediaSource, 'sourceopen', () => {
            this.emit(EVENTS.mseSourceOpen)
        })

        proxy(this.mediaSource, 'sourceclose', () => {
            this.emit(EVENTS.mseSourceClose);
        })
    }

    get state() {
        return this.mediaSource.readyState
    }

    get isStateOpen() {
        return this.state === MEDIA_SOURCE_STATE.open;
    }

    get isStateClosed() {
        return this.state === MEDIA_SOURCE_STATE.closed;
    }

    get isStateEnded() {
        return this.state === MEDIA_SOURCE_STATE.ended;
    }


    get duration() {
        return this.mediaSource.duration
    }

    set duration(duration) {
        this.mediaSource.duration = duration
    }

    decodeVideo(payload, ts, isIframe) {
        if (!this.hasInit) {
            if (isIframe && payload[1] === 0) {
                const videoCodec = (payload[0] & 0x0F);
                this.player.video.updateVideoInfo({
                    encTypeCode: videoCodec
                })
                this.hasInit = true;
                //
                const metaData = {
                    id: 1,
                    type: 'video',
                    timescale: 1000,
                    duration: 0,
                    avcc: payload.slice(5),
                    codecWidth: 0,
                    codecHeight: 0,
                    codec: 512
                }
                const metaBox = MP4.generateInitSegment(metaData);
                this.isAvc = true;
                this.appendBuffer(metaBox.buffer);
                this.cacheTrack = null;
                this.timeInit = false;
            }
        } else {
            // 真正的开始解码


        }


    }

    decodeAudio(payload, ts) {

    }


    appendBuffer(buffer) {
        const {
            debug,
            events: {proxy},
        } = player;
        if (this.sourceBuffer === null) {
            const codecs = this.isAvc ? MP4_CODECS.avc : MP4_CODECS.hev;
            this.sourceBuffer = this.mediaSource.addSourceBuffer(codecs);
            proxy(this.sourceBuffer, 'error', (error) => {
                this.emit(EVENTS.mseSourceBufferError, error);
            })
        }

        if (this.sourceBuffer.updating === false && this.isStateOpen) {
            this.sourceBuffer.appendBuffer(buffer);
            return;
        }

        if (this.isStateClosed) {
            this.emit(EVENTS.mseSourceBufferError, 'mediaSource is not attached to video or mediaSource is closed')
        } else if (this.isStateEnded) {
            this.emit(EVENTS.mseSourceBufferError, 'mediaSource is closed')
        } else {
            if (this.sourceBuffer.updating === true) {
                this.emit(EVENTS.mseSourceBufferBusy);
            }
        }
    }


    start() {

    }


    stop() {
        if (this.isStateOpen) {
            if (this.sourceBuffer) {
                this.sourceBuffer.abort();
            }
            this.endOfStream();
        }
    }

    removeBuffer(start, end) {
        if (this.isStateOpen) {
            try {
                this.sourceBuffer.remove(start, end)
            } catch (e) {

            }
        }
    }

    endOfStream() {
        if (this.isStateOpen) {
            this.mediaSource.endOfStream();
        }
    }


    destroy() {
        this.sourceBuffer = null;
        this.hasInit = false;
        this.isInitInfo = false;
    }
}
