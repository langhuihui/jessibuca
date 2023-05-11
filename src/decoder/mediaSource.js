import Emitter from "../utils/emitter";
import {EVENTS, EVENTS_ERROR, FRAG_DURATION, MEDIA_SOURCE_STATE, MP4_CODECS, VIDEO_ENC_CODE} from "../constant";
import MP4 from "../remux/fmp4-generator";
import {parseAVCDecoderConfigurationRecord,} from "../utils/h264";
import {now} from "../utils";

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
        this.dropping = false;
        this.firstRenderTime = null;
        this.mediaSourceAppendBufferError = false;
        this.mediaSourceAppendBufferFull = false;
        this.isDecodeFirstIIframe = false;
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

    destroy() {
        this.stop();
        this.mediaSource = null;
        this.mediaSourceOpen = false;
        this.sourceBuffer = null;
        this.hasInit = false;
        this.isInitInfo = false;
        this.sequenceNumber = 0;
        this.cacheTrack = null;
        this.timeInit = false;
        this.mediaSourceAppendBufferError = false;
        this.mediaSourceAppendBufferFull = false;
        this.isDecodeFirstIIframe = false;
        this.off();
        this.player.debug.log('MediaSource', 'destroy')
    }

    get state() {
        return this.mediaSource && this.mediaSource.readyState
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
        return this.mediaSource && this.mediaSource.duration
    }

    set duration(duration) {
        this.mediaSource.duration = duration
    }

    decodeVideo(payload, ts, isIframe, cts) {
        const player = this.player;

        if (!player) {
            return;
        }

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
                if (!player._times.decodeStart) {
                    player._times.decodeStart = now();
                }

                this._decodeConfigurationRecord(payload, ts, isIframe, videoCodec)
                this.hasInit = true;
            }
        } else {
            if (isIframe && payload[1] === 0) {
                let config = parseAVCDecoderConfigurationRecord(payload.slice(5))
                const videoInfo = this.player.video.videoInfo;
                if ((videoInfo && videoInfo.width && videoInfo.height) && (config && config.codecWidth && config.codecHeight) && (config.codecWidth !== videoInfo.width || config.codecHeight !== videoInfo.height)) {
                    this.player.debug.warn('MediaSource', `width or height is update, width ${videoInfo.width}-> ${config.codecWidth}, height ${videoInfo.height}-> ${config.codecHeight}`)
                    this.isInitInfo = false;
                    this.player.video.init = false;
                }
            }

            if (!this.isDecodeFirstIIframe && isIframe) {
                this.isDecodeFirstIIframe = true;
            }
            if (this.isDecodeFirstIIframe) {
                if (this.firstRenderTime === null) {
                    this.firstRenderTime = ts;
                }
                const dts = ts - this.firstRenderTime;

                this._decodeVideo(payload, dts, isIframe, cts);
            } else {
                this.player.debug.warn('MediaSource', 'decodeVideo isDecodeFirstIIframe false')
            }
        }
    }


    _decodeConfigurationRecord(payload, ts, isIframe, videoCodec) {
        let data = payload.slice(5);
        let config = {};

        config = parseAVCDecoderConfigurationRecord(data)

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
    _decodeVideo(payload, dts, isIframe, cts) {
        const player = this.player;
        let arrayBuffer = payload.slice(5);
        let bytes = arrayBuffer.byteLength;
        // player.debug.log('MediaSource', '_decodeVideo', ts);
        const $video = player.video.$videoElement;
        const videoBufferDelay = player._opt.videoBufferDelay;
        if ($video.buffered.length > 1) {
            this.removeBuffer($video.buffered.start(0), $video.buffered.end(0));
            this.timeInit = false;
        }
        if (this.dropping && dts - this.cacheTrack.dts > videoBufferDelay) {
            this.dropping = false;
            this.cacheTrack = {};
        } else if (this.cacheTrack && dts >= this.cacheTrack.dts) {
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
            player.updateStats({fps: true, ts: dts, buf: (player.demux && player.demux.delay) || 0})
            if (!player._times.videoStart) {
                player._times.videoStart = now();
                player.handlePlayToRenderTimes()
            }
        } else {
            player.debug.log('MediaSource', 'timeInit set false , cacheTrack = {}');
            this.timeInit = false;
            this.cacheTrack = {};
        }
        if (!this.cacheTrack) {
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
                // this.dropSourceBuffer(false)
            })
        }

        if (this.mediaSourceAppendBufferError) {
            debug.error('MediaSource', `this.mediaSourceAppendBufferError is true`);
            return;
        }

        if (this.mediaSourceAppendBufferFull) {
            debug.error('MediaSource', `this.mediaSourceAppendBufferFull is true`);
            return;
        }


        if (this.sourceBuffer.updating === false && this.isStateOpen) {
            try {
                this.sourceBuffer.appendBuffer(buffer);
            } catch (e) {
                debug.warn('MediaSource', 'this.sourceBuffer.appendBuffer()', e.code, e);
                if (e.code === 22) {
                    // QuotaExceededError
                    // The SourceBuffer is full, and cannot free space to append additional buffers
                    this.stop();
                    this.mediaSourceAppendBufferFull = true;
                    this.emit(EVENTS_ERROR.mediaSourceFull)
                } else if (e.code === 11) {
                    //     Failed to execute 'appendBuffer' on 'SourceBuffer': The HTMLMediaElement.error attribute is not null.
                    this.stop();
                    this.mediaSourceAppendBufferError = true;
                    this.emit(EVENTS_ERROR.mediaSourceAppendBufferError);
                } else {
                    debug.error('MediaSource', 'appendBuffer error', e)
                    this.player.emit(EVENTS.mseSourceBufferError, e);
                }
            }
            return;
        }

        if (this.isStateClosed) {
            this.player.emitError(EVENTS_ERROR.mseSourceBufferError, 'mediaSource is not attached to video or mediaSource is closed')
        } else if (this.isStateEnded) {
            this.player.emitError(EVENTS_ERROR.mseSourceBufferError, 'mediaSource is closed')
        } else {
            if (this.sourceBuffer.updating === true) {
                this.player.emit(EVENTS.mseSourceBufferBusy);
                // this.dropSourceBuffer(true);
            }
        }
    }

    stop() {
        this.abortSourceBuffer();
        this.removeSourceBuffer();
        this.endOfStream();
    }

    dropSourceBuffer(isDropping) {
        const $video = this.player.video.$videoElement;
        this.dropping = isDropping;
        if ($video.buffered.length > 0) {
            if ($video.buffered.end(0) - $video.currentTime > 1) {
                this.player.debug.warn('MediaSource', 'dropSourceBuffer', `$video.buffered.end(0) is ${$video.buffered.end(0)} - $video.currentTime ${$video.currentTime}`);
                $video.currentTime = $video.buffered.end(0);
            }
        }
    }


    removeBuffer(start, end) {

        if (this.isStateOpen && this.sourceBuffer.updating === false) {
            try {
                this.sourceBuffer.remove(start, end)
            } catch (e) {
                this.player.debug.warn('MediaSource', 'removeBuffer() error', e);
            }
        } else {
            this.player.debug.warn('MediaSource', 'removeBuffer() this.isStateOpen is', this.isStateOpen, 'this.sourceBuffer.updating', this.sourceBuffer.updating);
        }
    }

    endOfStream() {
        // fix: MediaSource endOfStream before demuxer initialization completes (before HAVE_METADATA) is treated as an error
        const $videoElement = this.player.video && this.player.video.$videoElement;
        if (this.isStateOpen && $videoElement && $videoElement.readyState >= 1) {
            try {
                this.mediaSource.endOfStream();
            } catch (e) {
                this.player.debug.warn('MediaSource', 'endOfStream() error', e);
            }
        }
    }

    abortSourceBuffer() {
        if (this.isStateOpen) {
            if (this.sourceBuffer) {
                this.sourceBuffer.abort();
                this.sourceBuffer = null;
            }
        }
    }

    removeSourceBuffer() {
        if (!this.isStateClosed) {
            if (this.mediaSource && this.sourceBuffer) {
                try {
                    this.mediaSource.removeSourceBuffer(this.sourceBuffer);
                } catch (e) {
                    this.player.debug.warn('MediaSource', 'removeSourceBuffer() error', e);
                }
            }
        }
    }


}
