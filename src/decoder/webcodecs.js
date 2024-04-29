import {formatVideoDecoderConfigure, noop, now} from "../utils";
import Emitter from "../utils/emitter";
import {ENCODED_VIDEO_TYPE, EVENTS, EVENTS_ERROR, VIDEO_ENC_CODE, WCS_ERROR} from "../constant";
import {parseAVCDecoderConfigurationRecord} from "../utils/h264";


export default class WebcodecsDecoder extends Emitter {
    constructor(player) {
        super();
        this.player = player;
        this.hasInit = false;
        this.isDecodeFirstIIframe = false;
        this.isInitInfo = false;
        this.decoder = null;
        this.initDecoder();
        player.debug.log('Webcodecs', 'init')
    }

    destroy() {
        if (this.decoder) {
            if (this.decoder.state !== 'closed') {
                this.decoder.close();
            }
            this.decoder = null;
        }

        this.hasInit = false;
        this.isInitInfo = false;
        this.isDecodeFirstIIframe = false;
        this.off();
        this.player.debug.log('Webcodecs', 'destroy')
    }

    initDecoder() {
        const _this = this;
        this.decoder = new VideoDecoder({
            output(videoFrame) {
                _this.handleDecode(videoFrame)
            },
            error(error) {
                _this.handleError(error)
            }
        })
    }

    handleDecode(videoFrame) {
        if (!this.isInitInfo) {
            this.player.video.updateVideoInfo({
                width: videoFrame.codedWidth,
                height: videoFrame.codedHeight
            })
            this.player.video.initCanvasViewSize();
            this.isInitInfo = true;
        }

        if (!this.player._times.videoStart) {
            this.player._times.videoStart = now();
            this.player.handlePlayToRenderTimes();
        }

        this.player.handleRender();
        this.player.video.render({
            videoFrame
        })

        this.player.updateStats({fps: true, ts: 0, buf: this.player.demux.delay})
    }

    handleError(error) {
        this.player.debug.error('Webcodecs', 'VideoDecoder handleError', error)
    }

    decodeVideo(payload, ts, isIframe) {
        // this.player.debug.log('Webcodecs decoder', 'decodeVideo', ts, isIframe);
        if (!this.hasInit) {
            if (isIframe && payload[1] === 0) {
                const videoCodec = (payload[0] & 0x0F);
                this.player.video.updateVideoInfo({
                    encTypeCode: videoCodec
                })

                // 如果解码出来的是
                if (videoCodec === VIDEO_ENC_CODE.h265) {
                    this.emit(EVENTS_ERROR.webcodecsH265NotSupport)
                    return;
                }
                if (!this.player._times.decodeStart) {
                    this.player._times.decodeStart = now();
                }

                const config = formatVideoDecoderConfigure(payload.slice(5));
                this.player.debug.log('Webcodecs', 'VideoDecoder configure', config)
                try {
                    this.decoder.configure(config);
                }
                catch (e){
                    this.player.debug.error('Webcodecs', 'VideoDecoder configure', e);
                    this.player.emit(EVENTS_ERROR.webcodecsConfigureError);
                    return;
                }
                this.hasInit = true;
            }
        } else {

            // check width or height change
            if (isIframe && payload[1] === 0) {
                let data = payload.slice(5);
                const config = parseAVCDecoderConfigurationRecord(data)
                const videoInfo = this.player.video.videoInfo;
                if (config.codecWidth !== videoInfo.width || config.codecHeight !== videoInfo.height) {
                    this.player.debug.log('Webcodecs', `width or height is update, width ${videoInfo.width}-> ${config.codecWidth}, height ${videoInfo.height}-> ${config.codecHeight}`)
                    this.player.emit(EVENTS_ERROR.webcodecsWidthOrHeightChange)
                    return;
                }
            }


            // fix : Uncaught DOMException: Failed to execute 'decode' on 'VideoDecoder': A key frame is required after configure() or flush().
            if (!this.isDecodeFirstIIframe && isIframe) {
                this.isDecodeFirstIIframe = true;
            }

            if (this.isDecodeFirstIIframe) {
                const chunk = new EncodedVideoChunk({
                    data: payload.slice(5),
                    timestamp: ts,
                    type: isIframe ? ENCODED_VIDEO_TYPE.key : ENCODED_VIDEO_TYPE.delta
                })
                this.player.emit(EVENTS.timeUpdate, ts);
                try {
                    if (this.isDecodeStateClosed()) {
                        this.player.debug.warn('Webcodecs', 'VideoDecoder isDecodeStateClosed true');
                        return;
                    }
                    this.decoder.decode(chunk);
                } catch (e) {
                    this.player.debug.error('Webcodecs', 'VideoDecoder', e)
                    if (e.toString().indexOf(WCS_ERROR.keyframeIsRequiredError) !== -1) {
                        this.player.emitError(EVENTS_ERROR.webcodecsDecodeError);
                    }
                    else if (e.toString().indexOf(WCS_ERROR.canNotDecodeClosedCodec) !== -1) {
                        this.player.emitError(EVENTS_ERROR.webcodecsDecodeError);
                    }
                }
            } else {
                this.player.debug.warn('Webcodecs', 'VideoDecoder isDecodeFirstIIframe false')
            }
        }
    }

    isDecodeStateClosed() {
        return this.decoder.state === 'closed';
    }
}
