import Emitter from "../utils/emitter";
import {EVENTS, MEDIA_SOURCE_STATE, MP4_CODECS} from "../constant";
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
        this.sequenceNumber = 0;

        player.video.$videoElement.src = window.URL.createObjectURL(this.mediaSource);
        const {
            debug,
            events: {proxy},
        } = player;


        proxy(this.mediaSource, 'sourceopen', () => {
            this.player.emit(EVENTS.mseSourceOpen)
        })

        proxy(this.mediaSource, 'sourceclose', () => {
            this.player.emit(EVENTS.mseSourceClose);
        })
        player.debug.log('MediaSource', 'init')
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
        const player = this.player;
        if (!this.hasInit) {
            if (isIframe && payload[1] === 0) {
                const videoCodec = (payload[0] & 0x0F);
                player.video.updateVideoInfo({
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
                    codecWidth: 960,
                    codecHeight: 540,
                    codec: 512
                }
                const metaBox = MP4.generateInitSegment(metaData);
                this.isAvc = true;
                this.appendBuffer(metaBox.buffer);
                this.sequenceNumber = 0;
                this.cacheTrack = null;
                this.timeInit = false;
                player.debug.log('MediaSource', 'generateInitSegment');
            }
        } else {
            let arrayBuffer = payload.slice(5);
            // 真正的开始解码
            let bytes = arrayBuffer.byteLength;
            let cts = 0;
            let dts = ts;
            let flags = isIframe;
            const $video = player.video.$videoElement;
            if ($video.buffered.length > 0) {
                this.removeBuffer($video.buffered.start(0), $video.buffered.end(0));
                this.timeInit = false;
            }
            if (this.cacheTrack && dts > this.cacheTrack.dts) {
                let mdatBytes = 8 + this.cacheTrack.size;
                let mdatbox = new Uint8Array(mdatBytes);
                mdatbox[0] = mdatBytes >>> 24 & 255;
                mdatbox[1] = mdatBytes >>> 16 & 255;
                mdatbox[2] = mdatBytes >>> 8 & 255;
                mdatbox[3] = mdatBytes & 255;
                mdatbox.set(MP4.types.mdat, 4);
                mdatbox.set(this.cacheTrack.data, 8);
                this.cacheTrack.duration = dts - this.cacheTrack.dts;
                let moofbox = MP4.moof(this.cacheTrack, this.cacheTrack.dts);
                let result = new Uint8Array(moofbox.byteLength + mdatbox.byteLength);
                result.set(moofbox, 0);
                result.set(mdatbox, moofbox.byteLength);
                this.appendBuffer(result.buffer)
                player.handleRender();
                player.debug.log('MediaSource', 'add render data');
                player.handleRender();
            } else {
                this.timeInit = false;
                this.cacheTrack = {};
            }
            this.cacheTrack.id = 1;
            this.cacheTrack.sn = ++this.sequenceNumber;
            this.cacheTrack.size = bytes;
            this.cacheTrack.dts = dts;
            this.cacheTrack.cts = cts;
            this.cacheTrack.data = arrayBuffer;
            this.cacheTrack.flags = {
                isLeading: 0,
                dependsOn: flags ? 2 : 1,
                isDependedOn: flags ? 1 : 0,
                hasRedundancy: 0,
                isNonSync: flags ? 0 : 1
            }

            if (!this.timeInit && $video.buffered.length === 1) {
                this.timeInit = true;
                $video.currentTime = $video.buffered.end(0);
            }

            if (!this.isInitInfo) {
                player.video.updateVideoInfo({
                    width: $video.videoWidth,
                    height: $video.videoHeight
                })
                player.video.initCanvasViewSize();
                this.isInitInfo = true;
            }

        }
    }

    decodeAudio(payload, ts) {

    }


    appendBuffer(buffer) {
        const {
            debug,
            events: {proxy},
        } = this.player;
        if (this.sourceBuffer === null) {
            const codecs = this.isAvc ? MP4_CODECS.avc : MP4_CODECS.hev;
            this.sourceBuffer = this.mediaSource.addSourceBuffer(codecs);
            proxy(this.sourceBuffer, 'error', (error) => {
                this.player.emit(EVENTS.mseSourceBufferError, error);
            })
        }

        if (this.sourceBuffer.updating === false && this.isStateOpen) {
            this.player.debug.log('sourceBuffer updating is false , state is', this.state)
            this.sourceBuffer.appendBuffer(buffer);
            return;
        }

        if (this.isStateClosed) {
            this.player.emit(EVENTS.mseSourceBufferError, 'mediaSource is not attached to video or mediaSource is closed')
        } else if (this.isStateEnded) {
            this.player.emit(EVENTS.mseSourceBufferError, 'mediaSource is closed')
        } else {
            if (this.sourceBuffer.updating === true) {
                this.player.emit(EVENTS.mseSourceBufferBusy);
            }
        }
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
        this.stop();
        this.sourceBuffer = null;
        this.hasInit = false;
        this.isInitInfo = false;
    }
}
