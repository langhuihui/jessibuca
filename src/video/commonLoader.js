import Emitter from "../utils/emitter";
import {CONTROL_HEIGHT, EVENTS, VIDEO_ENC_TYPE} from "../constant";

export default class CommonLoader extends Emitter {
    constructor() {
        super();
        this.init = false;
    }

    //
    updateVideoInfo(data) {
        if (data.encTypeCode) {
            this.videoInfo.encType = VIDEO_ENC_TYPE[data.encTypeCode];
        }

        if (data.width) {
            this.videoInfo.width = data.width;
        }

        if (data.height) {
            this.videoInfo.height = data.height;
        }

        // video 基本信息
        if (this.videoInfo.encType && this.videoInfo.height && this.videoInfo.width && !this.init) {
            this.player.emit(EVENTS.videoInfo, this.videoInfo);
            this.init = true;
        }
    }

    resize() {
        this.player.debug.log('canvasVideo', 'resize');
        const option = this.player._opt;
        const width = this.player.width;
        let height = this.player.height;
        if (option.hasControl) {
            height -= CONTROL_HEIGHT;
        }
        let resizeWidth = this.$videoElement.width;
        let resizeHeight = this.$videoElement.height;
        const rotate = option.rotate;
        let left = ((width - resizeWidth) / 2)
        let top = ((height - resizeHeight) / 2)
        if (rotate === 270 || rotate === 90) {
            resizeWidth = this.$videoElement.height;
            resizeHeight = this.$videoElement.width;
        }

        const wScale = width / resizeWidth;
        const hScale = height / resizeHeight;

        let scale = wScale > hScale ? hScale : wScale;
        //
        if (!option.isResize) {
            if (wScale !== hScale) {
                scale = wScale + ',' + hScale;
            }
        }
        //
        if (option.isFullResize) {
            scale = wScale > hScale ? wScale : hScale;
        }
        let transform = "scale(" + scale + ")";

        if (rotate) {
            transform += ' rotate(' + rotate + 'deg)'
        }

        this.$videoElement.style.transform = transform;
        this.$videoElement.style.left = left + "px"
        this.$videoElement.style.top = top + "px"
    }
}
