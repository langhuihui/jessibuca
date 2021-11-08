import {DEFAULT_PLAYER_OPTIONS} from "../constant";
import Debug from "../utils/debug";
import Events from "../utils/events";
import property from './property';
import events from './events';

export default class Player {
    constructor(container, options) {
        this.$container = container;
        this._opt = Object.assign({}, DEFAULT_PLAYER_OPTIONS, options)


    }


    play() {

    }


    destroy() {

    }

}
