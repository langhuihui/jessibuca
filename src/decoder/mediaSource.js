import Emitter from "../utils/emitter";
import {
    EVENTS,
    EVENTS_ERROR,
    FRAG_DURATION,
    MEDIA_SOURCE_STATE,
    MEDIA_SOURCE_UPDATE_END_TIMEOUT,
    MP4_CODECS,
    VIDEO_ENC_CODE
} from "../constant";
import MP4 from "../remux/fmp4-generator";
import {parseAVCDecoderConfigurationRecord} from "../utils/h264";
import {parseHEVCDecoderConfigurationRecord} from "../utils/h265";
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
        this.bufferList = [];
        this.dropping = false;
        this.mediaSourceAppendBufferError = false;
        this.mediaSourceAppendBufferFull = false;
        this.mediaSourceBufferListLarge = false;
        this.mediaSourceLocked = false;
        this.mediaSourceAppendBufferEndTimeout = false;
        this.checkAppendBufferEndTimeout = null;
        this.mediaSourceObjectURL = window.URL.createObjectURL(this.mediaSource);
        this.player.video.$videoElement.src = this.mediaSourceObjectURL;
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
        this.bufferList = [];
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
        this.mediaSourceBufferListLarge = false;
        this.mediaSourceLocked = false;
        this.mediaSourceAppendBufferEndTimeout = false;
        this.checkAppendBufferEndTimeout = null;
        if (this.mediaSourceObjectURL) {
            window.URL.revokeObjectURL(this.mediaSourceObjectURL);
            this.mediaSourceObjectURL = null;
        }
        this.off();
        this.player.debug.log('MediaSource', 'destroy')
        this.player = null;
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
                if (!player._times.decodeStart) {
                    player._times.decodeStart = now();
                }

                this._decodeConfigurationRecord(payload, ts, isIframe, videoCodec)
                this.hasInit = true;
            }
        } else {
            this._decodeVideo(payload, ts, isIframe);
        }
    }

    _doAppendBuffer() {
        const bufferItem = this.bufferList.shift();
        if (bufferItem) {
            if (this.mediaSourceAppendBufferError || this.mediaSourceAppendBufferFull || this.mediaSourceBufferListLarge || this.mediaSourceAppendBufferEndTimeout) {
                return;
            }
            this.appendBuffer(bufferItem);
        }
    }

    _doPendingBuffer(bufferItem) {
        this.bufferList.push(bufferItem);
        this.player.debug.log('MediaSource', '_doPendingBuffer buffer list is', this.bufferList.length);
        if (this.bufferList.length > 10) {
            this.player.debug.warn('MediaSource', '_doPendingBuffer buffer list is', this.bufferList.length);
        }

        if (this.bufferList.length > 500) {
            this.player.debug.error('MediaSource', '_doPendingBuffer buffer list is', this.bufferList.length);
            this.mediaSourceBufferListLarge = true;
            this.stop();
            this.emit(EVENTS_ERROR.mediaSourceBufferListLarge);
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
    _decodeVideo(payload, ts, isIframe) {
        const player = this.player;
        let arrayBuffer = payload.slice(5);
        let bytes = arrayBuffer.byteLength;
        let cts = 0;
        let dts = ts;
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
            if (!player._times.videoStart) {
                player._times.videoStart = now();
                player.handlePlayToRenderTimes()
            }
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

        if (this.isStateClosed) {
            debug.warn('MediaSource', 'mediaSource is not attached to video or mediaSource is closed');
            this.player.emit(EVENTS_ERROR.mseSourceBufferError, 'mediaSource is not attached to video or mediaSource is closed')
            return;
        } else if (this.isStateEnded) {
            debug.warn('MediaSource', 'mediaSource is closed');
            this.player.emit(EVENTS_ERROR.mseSourceBufferError, 'mediaSource is closed')
            return;
        } else {
            if (this.sourceBuffer && this.sourceBuffer.updating === true) {
                this.player.emit(EVENTS.mseSourceBufferBusy);
                this._doPendingBuffer(buffer);
                return;
            }
        }

        if (!this.isStateOpen) {
            debug.warn('MediaSource', 'appendBuffer this.state is not open ,is ', this.state);
            this._doPendingBuffer(buffer);
            return;
        }

        if (this.mediaSourceLocked) {
            this._doPendingBuffer(buffer);
            return;
        }

        if (this.sourceBuffer === null && this.mediaSource.sourceBuffers.length === 0) {
            this.sourceBuffer = this.mediaSource.addSourceBuffer(MP4_CODECS.avc);
            proxy(this.sourceBuffer, 'error', (error) => {
                this.player.emit(EVENTS.mseSourceBufferError, error);
            })
            proxy(this.sourceBuffer, 'updateend', () => {
                debug.log('MediaSource', 'this.sourceBuffer updateend');
                this.mediaSourceLocked = false;
                this._startCheckAppendBufferEndTimeout();
                this._doAppendBuffer();
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

        if (this.mediaSourceBufferListLarge) {
            debug.error('MediaSource', `this.mediaSourceBufferListLarge is true`);
            return;
        }

        if (this.mediaSourceAppendBufferEndTimeout) {
            debug.error('MediaSource', `this.mediaSourceAppendBufferEndTimeout is true`);
            return;
        }


        if (this.sourceBuffer && this.sourceBuffer.updating === false ) {
            if (this.sourceBuffer.appendBuffer) {
                try {
                    this.mediaSourceLocked = true;
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
                        this.player.video.play();
                    } else {
                        debug.error('MediaSource', 'appendBuffer error', e)
                        this.player.emit(EVENTS.mseSourceBufferError, e);
                    }
                }

            } else {
                debug.warn('MediaSource', 'this.sourceBuffer.appendBuffer function is undefined');
            }
        } else {
            debug.warn('MediaSource', 'this.sourceBuffer is null or this.sourceBuffer.updating is false');
        }
    }

    stop() {
        this.abortSourceBuffer();
        this.removeSourceBuffer();
        this.endOfStream();
        this.bufferList = [];
    }

    dropSourceBuffer(isDropping) {
        const $video = this.$videoElement;
        this.dropping = isDropping;
        // console.log('$video.buffered.length', $video.buffered.length, '$video.buffered.end(0)', $video.buffered.end(0), '$video.currentTime', $video.currentTime)
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
                this.player.debug.warn('MediaSource', 'removeBuffer() error', e);
            }
        } else {
            this.player.debug.warn('MediaSource', 'removeBuffer() this.isStateOpen is', this.isStateOpen, 'this.sourceBuffer.updating', this.sourceBuffer.updating);
        }
    }

    endOfStream() {
        if (this.isStateOpen) {
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

    _startCheckAppendBufferEndTimeout() {
        this._stopCheckAppendBufferEndTimeout();
        const {
            debug,
        } = this.player;
        this.checkAppendBufferEndTimeout = setTimeout(() => {
            debug.error('MediaSource', '_startCheckAppendBufferEndTimeout timeout');
            this.mediaSourceAppendBufferEndTimeout = true;
            this.stop();
            this.emit(EVENTS_ERROR.mediaSourceAppendBufferEndTimeout);
        }, MEDIA_SOURCE_UPDATE_END_TIMEOUT)
    }

    _stopCheckAppendBufferEndTimeout() {
        if (this.checkAppendBufferEndTimeout) {
            clearTimeout(this.checkAppendBufferEndTimeout);
            this.checkAppendBufferEndTimeout = null;
        }
    }
}
