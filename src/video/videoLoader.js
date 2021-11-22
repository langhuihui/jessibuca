import Emitter from "../utils/emitter";
import {CONTROL_HEIGHT} from "../constant";

export default class VideoLoader extends Emitter {
    constructor(player) {
        super();
        this.player = player;
        const $videoElement = document.createElement('video');
        $videoElement.muted = true;
        $videoElement.style.position = "absolute";
        $videoElement.style.top = 0;
        $videoElement.style.left = 0;
        player.$container.appendChild($videoElement);
        this.$videoElement = $videoElement;
        this.resize();
    }

    resize() {
        this.$videoElement.width = this.player.width;
        this.$videoElement.height = this.player._opt.hasControl ? this.player.height - CONTROL_HEIGHT : this.player.height;
    }

    destroy() {
        this.player.$container.removeChild(this.$videoElement);
        this.player.debug.log('commonVideo', 'destroy');
        this.player = null;
    }

}
