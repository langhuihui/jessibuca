import template from "./template";
import observer from './observer';
import property from './property';
import events from './events';
import './style.scss'
import hotkey from "./hotkey";

export default class Control {
    constructor(player) {
        this.player = player;
        template(player, this);
        property(player, this);
        observer(player, this);
        events(player, this);
        if (player._opt.hotKey) {
            hotkey(player, this);
        }
        this.player.debug.log('Control', 'init');
    }

    destroy() {
        if (this.$poster) {
            this.player.$container.removeChild(this.$poster);
        }
        if (this.$loading) {
            this.player.$container.removeChild(this.$loading);
        }
        if (this.$controls) {
            this.player.$container.removeChild(this.$controls);
        }

        if(this.$recording){
            this.player.$container.removeChild(this.$recording);
        }

        if(this.$playBig){
            this.player.$container.removeChild(this.$playBig);
        }

        this.player.debug.log('control', 'destroy');
    }

    autoSize() {
        const player = this.player;
        player.$container.style.padding = '0 0';
        const playerWidth = player.width;
        const playerHeight = player.height;
        const playerRatio = playerWidth / playerHeight;
        const canvasWidth = player.video.$videoElement.width;
        const canvasHeight = player.video.$videoElement.height;
        const canvasRatio = canvasWidth / canvasHeight;
        if (playerRatio > canvasRatio) {
            const padding = (playerWidth - playerHeight * canvasRatio) / 2;
            player.$container.style.padding = `0 ${padding}px`;
        } else {
            const padding = (playerHeight - playerWidth / canvasRatio) / 2;
            player.$container.style.padding = `${padding}px 0`;
        }
    }


}
