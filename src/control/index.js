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

    }

    destroy() {
        this.player.debug.log('control', 'destroy');
        if (this.$poster) {
            this.player.$container.removeChild(this.$poster);
        }
        this.player.$container.removeChild(this.$loading);
        this.player.$container.removeChild(this.$ptzControl);
        this.player.$container.removeChild(this.$controls);
        this.player = null;
    }
}
