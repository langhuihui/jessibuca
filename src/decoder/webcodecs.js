import {formatVideoDecoderConfigure, noop, now} from "../utils";
import Emitter from "../utils/emitter";
import {ENCODED_VIDEO_TYPE, EVENTS, EVENTS_ERROR, VIDEO_ENC_CODE} from "../constant";


export default class WebcodecsDecoder extends Emitter {
    constructor(player) {
        super();
        this.player = player;
        this.hasInit = false;
        this.isInitInfo = false;
        this.decoder = null;
        this.initDecoder();
        player.debug.log('Webcodecs', 'init')
    }

    destroy() {
        if (this.decoder) {
            this.decoder.close();
            this.decoder = null;
        }

        this.hasInit = false;
        this.isInitInfo = false;
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

        // release resource
        setTimeout(function () {
            if (videoFrame.close) {
                videoFrame.close()
            } else {
                videoFrame.destroy()
            }
        }, 100)
    }

    handleError(error) {
        this.player.debug.log('Webcodecs', 'VideoDecoder handleError', error)
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
                this.decoder.configure(config);
                this.hasInit = true;
            }
        } else {
            const chunk = new EncodedVideoChunk({
                data: payload.slice(5),
                timestamp: ts,
                type: isIframe ? ENCODED_VIDEO_TYPE.key : ENCODED_VIDEO_TYPE.delta
            })
            this.decoder.decode(chunk);
        }
    }


}
