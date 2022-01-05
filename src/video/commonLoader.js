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


}
