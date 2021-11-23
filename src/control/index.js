import template from "./template";
import observer from './observer';
import property from './property';
import events from './events';
import './style.scss'

export default class Control {
    constructor(player) {
        this.player = player;
        template(player, this);
        observer(player, this);
        property(player, this);
        events(player, this);
        this.player.debug.log('Control', 'init');
    }

    autoSize() {
        const player = this.player;
        player.$container.style.padding = '0 0';
        const playerWidth = player.width;
        const playerHeight = player.height;
        const playerRatio = playerWidth / playerHeight;
        const canvasWidth = player.audio.$videoElement.width;
        const canvasHeight = player.audio.$videoElement.height;
        const canvasRatio = canvasWidth / canvasHeight;
        if (playerRatio > canvasRatio) {
            const padding = (playerWidth - playerHeight * canvasRatio) / 2;
            player.$container.style.padding = `0 ${padding}px`;
        } else {
            const padding = (playerHeight - playerWidth / canvasRatio) / 2;
            player.$container.style.padding = `${padding}px 0`;
        }
    }

    destroy() {
        this.player.debug.log('control', 'destroy');
        if (this.$poster) {
            this.player.$container.removeChild(this.$poster);
        }
        this.player.$container.removeChild(this.$loading);
        if (this.$controls) {
            this.player.$container.removeChild(this.$controls);
        }
        this.player = null;
    }
}
