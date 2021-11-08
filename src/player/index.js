import {DEFAULT_PLAYER_OPTIONS} from "../constant";
import Debug from "../utils/debug";
import Events from "../utils/events";
import property from './property';
import events from './events';
import {supportWCS} from "../utils";


export default class Player {
    constructor(container, options) {
        this.$container = container;
        this._opt = Object.assign({}, DEFAULT_PLAYER_OPTIONS, options)


        if (this._opt.useWCS) {
            this._opt.useWCS = supportWCS();
        }


        property(this);
        events(this);
        this._loading = true;
        this._hasLoaded = false;

        this.debug = new Debug(this);
        this.events = new Events(this);


        this.debug.log('options', this._opt);
    }


    play() {

    }


    destroy() {

    }

}
