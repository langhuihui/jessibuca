import Emitter from "../utils/emitter";
import {EVENTS, MEDIA_SOURCE_STATE, MP4_CODECS} from "../constant";
import MP4 from "../remux/mp4-generator";
import {formatMp4VideoCodec, readSPS} from "../utils";
import {parseAVCDecoderConfigurationRecord} from "../utils/h264";

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
        this.codec = null;

        player.video.$videoElement.src = window.URL.createObjectURL(this.mediaSource);
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


        proxy(player.video.$videoElement, 'waiting', () => {
            this.player.emit(EVENTS.videoWaiting);
        })

        proxy(player.video.$videoElement, 'timeupdate', () => {
            this.player.emit(EVENTS.videoTimeUpdate);
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

    //
    decodeVideo(payload, ts, isIframe) {
        const player = this.player;
        if (!this.hasInit) {
            //
            if (isIframe && payload[1] === 0) {
                const videoCodec = (payload[0] & 0x0F);
                player.video.updateVideoInfo({
                    encTypeCode: videoCodec
                })
                this.hasInit = true;
                player.debug.log('MediaSource', 'decodeVideo hasInit set true');
                let data = payload.slice(5);
                const avcConfig = parseAVCDecoderConfigurationRecord(data);
                console.log(avcConfig);
                this.codec = formatMp4VideoCodec(avcConfig.codec);
                // 这一步肯定是成功的。
                const metaData = {
                    id: 1,
                    type: 'video',
                    timescale: 1000,
                    duration: 0,
                    // codec: avcConfig.codec,
                    avcc: data,
                    codecWidth: avcConfig.codecWidth,
                    codecHeight: avcConfig.codecHeight,
                }
                // ftyp
                const metaBox = MP4.generateInitSegment(metaData);
                this.isAvc = true;
                this.appendBuffer(metaBox.buffer);
                this.sequenceNumber = 0;
                this.cacheTrack = null;
                this.timeInit = false;
            }
        } else {
            let arrayBuffer = payload.slice(5);
            // 真正的开始解码
            let bytes = arrayBuffer.byteLength;
            let cts = 0;
            let dts = ts;

            const $video = player.video.$videoElement;
            player.debug.log('MediaSource', 'decodeVideo', `$video.buffered.length:${$video.buffered.length}, this.sourceBuffer.buffered:${this.sourceBuffer.buffered.length},bytes:${bytes},cts:${cts},dts:${ts},flag:${isIframe}`);

            if ($video.buffered.length > 1) {
                this.removeBuffer($video.buffered.start(0), $video.buffered.end(0));
                this.timeInit = false;
            }
            player.debug.log('MediaSource', `cacheTrack.dts:${this.cacheTrack && this.cacheTrack.dts}`);
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
                player.debug.log('MediaSource', 'add render data');
            } else {
                player.debug.log('MediaSource', 'timeInit set false , cacheTrack = {}');
                this.timeInit = false;
                this.cacheTrack = {};
            }

            this.cacheTrack.id = 1;
            // this.cacheTrack.sequenceNumber = ++this.sequenceNumber;
            this.cacheTrack.sn = ++this.sequenceNumber;
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
    }

    decodeAudio(payload, ts) {

    }


    appendBuffer(buffer) {
        const {
            debug,
            events: {proxy},
        } = this.player;

        if (this.sourceBuffer === null && this.codec) {
            const codecs = MP4_CODECS.avc;
            // const codecs = this.codec;
            this.sourceBuffer = this.mediaSource.addSourceBuffer(codecs);
            proxy(this.sourceBuffer, 'error', (error) => {
                this.player.emit(EVENTS.mseSourceBufferError, error);
            })
        }

        if (this.sourceBuffer.updating === false && this.isStateOpen) {
            this.player.debug.log('MediaSource', 'appendBuffer')
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

    // todo: 待定，不知道是啥方法
    xxx(flag) {
        const video = this.player.video;
        const $video = video.$videoElement;
        // 不懂。。。。。。
        $video.drop = flag;

        if ($video.buffered.length > 0) {
            if ($video.buffered.end(0) - $video.currentTime > 1) {
                $video.currentTime = $video.buffered.end(0);
            }
        }
    }


    removeBuffer(start, end) {
        if (this.isStateOpen) {
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
        this.mediaSource = null;
        this.mediaSourceOpen = false;
        this.sourceBuffer = null;
        this.hasInit = false;
        this.isInitInfo = false;
        this.sequenceNumber = 0;
        this.cacheTrack = null;
        this.timeInit = false;
        this.codec = null;
    }
}
