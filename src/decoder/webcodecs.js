import {formatVideoDecoderConfigure, noop} from "../utils";
import Emitter from "../utils/emitter";
import {EVENTS} from "../constant";


export default class WebcodecsDecoder extends Emitter {
    constructor(player) {
        super();
        this.player = player;
        this.hasInit = false;
        this.isInitInfo = false;
        this.decoder = null;
        this.initDecoder();
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
        this.player.video.render({
            videoFrame
        })
        //
        setTimeout(function () {
            if (videoFrame.close) {
                videoFrame.close()
            } else {
                videoFrame.destroy()
            }
        }, 100)
    }

    handleError(error) {

    }

    decodeVideo(payload, ts, isIframe) {
        if (!this.hasInit) {
            if (isIframe && payload[1] === 0) {
                const videoCodec = (payload[0] & 0x0F);
                this.player.emit(EVENTS.videoInfo, {
                    encTypeCode: videoCodec
                })
                const config = formatVideoDecoderConfigure(payload.slice(5));
                this.decoder.configure(config);
                this.hasInit = true;
            }
        } else {
            const chunk = new EncodedVideoChunk({
                data: payload.slice(5),
                timestamp: ts,
                type: isIframe ? 'key' : 'delta'
            })
            this.decoder.decode(chunk);
        }
    }


    destroy() {
        this.decoder.close();
        this.decoder = null;

    }
}
