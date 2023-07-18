import template from "./template";
import observer from './observer';
import property from './property';
import events from './events';
import './style.scss'
import hotkey from "./hotkey";
import {removeElement} from "../utils";

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
            const result = removeElement(this.$poster);
            if(!result){
                const $poster = this.player.$container.querySelector('.jessibuca-poster');
                if ($poster && this.player.$container) {
                    this.player.$container.removeChild($poster);
                }
            }
        }
        if (this.$loading) {
            const result = removeElement(this.$loading)
            if (!result) {
                const $loading = this.player.$container.querySelector('.jessibuca-loading');
                if ($loading && this.player.$container) {
                    this.player.$container.removeChild($loading);
                }
            }
        }
        if (this.$controls) {
            const result = removeElement(this.$controls)
            if (!result) {
                const $controls = this.player.$container.querySelector('.jessibuca-controls');
                if ($controls && this.player.$container) {
                    this.player.$container.removeChild($controls);
                }
            }
        }

        if (this.$recording) {
            const result = removeElement(this.$recording)
            if (!result) {
                const $recording = this.player.$container.querySelector('.jessibuca-recording');
                if ($recording && this.player.$container) {
                    this.player.$container.removeChild($recording);
                }
            }
        }

        if (this.$playBig) {
            const result = removeElement(this.$playBig)
            if (!result) {
                const $playBig = this.player.$container.querySelector('.jessibuca-play-big');
                if ($playBig && this.player.$container) {
                    this.player.$container.removeChild($playBig);
                }
            }
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
