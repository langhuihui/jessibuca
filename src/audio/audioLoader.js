import Emitter from "../utils/emitter";
import {clamp} from "../utils";
import {EVENTS} from "../constant";

export default class AudioLoader extends Emitter {

    constructor(player) {
        super();
        this.player = player;
        this.$video = player.video.$videoElement;
        this.player.debug.log('Audio', 'init');

    }

    destroy() {
        this.off();
        this.player.debug.log('Audio', 'destroy');
    }

    get volume() {
        return this.$video.volume;
    }

    get isMute() {
        return this.$video.volume === 0;
    }

    mute(muted) {
        this.setVolume(muted ? 0 : (this.player.lastVolume || 0.5));
    }

    setVolume(volume) {
        volume = parseFloat(volume);
        if (isNaN(volume)) {
            return;
        }
        volume = clamp(volume, 0, 1);
        // 值从0.0（静音）到1.0（最大音量）。
        this.$video.volume = volume;
        this.player.emit(EVENTS.volumechange, this.player.volume);
    }


}
