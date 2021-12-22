import Emitter from "../utils/emitter";
import {EVENTS, EVENTS_ERROR, FRAG_DURATION, MEDIA_SOURCE_STATE, MP4_CODECS, VIDEO_ENC_CODE} from "../constant";
import MP4 from "../remux/fmp4-generator";
import {parseAVCDecoderConfigurationRecord} from "../utils/h264";
import {parseHEVCDecoderConfigurationRecord} from "../utils/h265";

export default class MseDecoder extends Emitter {
    constructor(player) {
        super();
        this.player = player;
        this.isAvc = true;
        this.mediaSource = new window.MediaSource();
        this.sourceBuffer = null;
        this.hasInit = false;
        this.isInitInfo = false;
        this.cacheTrack = {};
        this.timeInit = false;
        this.sequenceNumber = 0;
        this.mediaSourceOpen = false;
        this.bufferList = [];
        this.dropping = false;
        this.player.video.$videoElement.src = window.URL.createObjectURL(this.mediaSource);
        const {
            debug,
            events: {proxy},
        } = player;


        proxy(this.mediaSource, 'sourceopen', () => {
            this.mediaSourceOpen = true;
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

                // 如果解码出来的是
                if (videoCodec === VIDEO_ENC_CODE.h265) {
                    this.emit(EVENTS_ERROR.mediaSourceH265NotSupport)
                    return;
                }

                this._decodeConfigurationRecord(payload, ts, isIframe, videoCodec)
                this.hasInit = true;
            }
        } else {
            this._decodeVideo(payload, ts, isIframe);
        }
    }

    _doDecode() {
        const bufferItem = this.bufferList.shift();
        if (bufferItem) {
            this._decodeVideo(bufferItem.payload, bufferItem.ts, bufferItem.isIframe);
        }
    }


    _decodeConfigurationRecord(payload, ts, isIframe, videoCodec) {
        let data = payload.slice(5);
        let config = {};

        if (videoCodec === VIDEO_ENC_CODE.h264) {
            config = parseAVCDecoderConfigurationRecord(data)
        } else if (videoCodec === VIDEO_ENC_CODE.h265) {
            config = parseHEVCDecoderConfigurationRecord(data);
        }
        const metaData = {
            id: 1, // video tag data
            type: 'video',
            timescale: 1000,
            duration: 0,
            avcc: data,
            codecWidth: config.codecWidth,
            codecHeight: config.codecHeight,
            videoType: config.videoType
        }
        // ftyp
        const metaBox = MP4.generateInitSegment(metaData);
        this.isAvc = true;
        this.appendBuffer(metaBox.buffer);
        this.sequenceNumber = 0;
        this.cacheTrack = null;
        this.timeInit = false;
    }

    //
    _decodeVideo(payload, ts, isIframe) {
        const player = this.player;
        let arrayBuffer = payload.slice(5);
        let bytes = arrayBuffer.byteLength;
        let cts = 0;
        let dts = ts;

        const $video = player.video.$videoElement;

        if ($video.buffered.length > 1) {
            this.removeBuffer($video.buffered.start(0), $video.buffered.end(0));
            this.timeInit = false;
        }
        if ($video.drop && dts - this.cacheTrack.dts > 1000) {
            $video.drop = false;
            this.cacheTrack = {};
        } else if (this.cacheTrack && dts > this.cacheTrack.dts) {
            // 需要额外加8个size
            let mdatBytes = 8 + this.cacheTrack.size;
            let mdatbox = new Uint8Array(mdatBytes);
            mdatbox[0] = mdatBytes >>> 24 & 255;
            mdatbox[1] = mdatBytes >>> 16 & 255;
            mdatbox[2] = mdatBytes >>> 8 & 255;
            mdatbox[3] = mdatBytes & 255;
            mdatbox.set(MP4.types.mdat, 4);
            mdatbox.set(this.cacheTrack.data, 8);

            this.cacheTrack.duration = dts - this.cacheTrack.dts;
            // moof
            let moofbox = MP4.moof(this.cacheTrack, this.cacheTrack.dts);
            let result = new Uint8Array(moofbox.byteLength + mdatbox.byteLength);
            result.set(moofbox, 0);
            result.set(mdatbox, moofbox.byteLength);
            // appendBuffer
            this.appendBuffer(result.buffer)
            player.handleRender();
            player.updateStats({fps: true, ts: ts, buf: player.demux.delay})
        } else {
            player.debug.log('MediaSource', 'timeInit set false , cacheTrack = {}');
            this.timeInit = false;
            this.cacheTrack = {};
        }

        this.cacheTrack.id = 1;
        this.cacheTrack.sequenceNumber = ++this.sequenceNumber;
        this.cacheTrack.size = bytes;
        this.cacheTrack.dts = dts;
        this.cacheTrack.cts = cts;
        this.cacheTrack.isKeyframe = isIframe;
        this.cacheTrack.data = arrayBuffer;
        //
        this.cacheTrack.flags = {
            isLeading: 0,
            dependsOn: isIframe ? 2 : 1,
            isDependedOn: isIframe ? 1 : 0,
            hasRedundancy: 0,
            isNonSync: isIframe ? 0 : 1
        }

        //
        if (!this.timeInit && $video.buffered.length === 1) {
            player.debug.log('MediaSource', 'timeInit set true');
            this.timeInit = true;
            $video.currentTime = $video.buffered.end(0);
        }

        if (!this.isInitInfo && $video.videoWidth > 0 && $video.videoHeight > 0) {
            player.debug.log('MediaSource', `updateVideoInfo: ${$video.videoWidth},${$video.videoHeight}`);
            player.video.updateVideoInfo({
                width: $video.videoWidth,
                height: $video.videoHeight
            })
            player.video.initCanvasViewSize();
            this.isInitInfo = true;
        }
    }

    appendBuffer(buffer) {
        const {
            debug,
            events: {proxy},
        } = this.player;

        if (this.sourceBuffer === null) {
            this.sourceBuffer = this.mediaSource.addSourceBuffer(MP4_CODECS.avc);
            proxy(this.sourceBuffer, 'error', (error) => {
                this.player.emit(EVENTS.mseSourceBufferError, error);
            })
        }

        if (this.sourceBuffer.updating === false && this.isStateOpen) {
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
                this.dropSourceBuffer(true);
            }
        }
    }

    stop() {
        if (this.isStateOpen) {
            if (this.sourceBuffer) {
                this.sourceBuffer.abort();
            }
        }
        this.endOfStream();
    }

    dropSourceBuffer(flag) {
        const video = this.player.video;
        const $video = video.$videoElement;
        this.dropping = flag;
        if ($video.buffered.length > 0) {
            if ($video.buffered.end(0) - $video.currentTime > 1) {
                $video.currentTime = $video.buffered.end(0);
            }
        }
    }


    removeBuffer(start, end) {

        if (this.isStateOpen && this.sourceBuffer.updating === false) {
            try {
                this.sourceBuffer.remove(start, end)
            } catch (e) {
                console.error(e)
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
        this.bufferList = [];
        this.mediaSource = null;
        this.mediaSourceOpen = false;
        this.sourceBuffer = null;
        this.hasInit = false;
        this.isInitInfo = false;
        this.sequenceNumber = 0;
        this.cacheTrack = null;
        this.timeInit = false;
        this.off();
        this.player.debug.log('MediaSource', 'destroy')
    }
}
