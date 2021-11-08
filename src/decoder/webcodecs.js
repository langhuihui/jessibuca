import {formatVideoDecoderConfigure, noop} from "../utils";


export default class WebcodecsDecoder {
    constructor({initInfoCallback, videoFrameCallback}) {
        this.hasInit = false;
        this.isInitInfo = false;
        this.decoder = null;
        this.initInfoCallback = initInfoCallback || noop;
        this.videoFrameCallback = videoFrameCallback || noop;
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
        if (!this.isEmitInfo) {
            this.initInfoCallback({
                width: videoFrame.codedWidth,
                height: videoFrame.codedHeight
            })
            this.isEmitInfo = true;
        }
        this.videoFrameCallback(videoFrame);
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

    decode(payload, ts) {
        const isIframe = payload[0] >> 4 === 1;
        if (!this.hasInit) {
            if (isIframe && payload[1] === 0) {
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
        this.decoder = null;

    }
}
