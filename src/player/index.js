import {DEFAULT_PLAYER_OPTIONS} from "../constant";
import Debug from "../utils/debug";
import Events from "../utils/events";
import property from './property';
import events from './events';
import {supportOffscreen, supportWCS} from "../utils";
import Video from "../video";
import Stream from "../stream";
import Recorder from "../recorder";
import DecoderWorker from "../worker";
import Emitter from "../utils/emitter";
import Demux from "../demux";

export default class Player extends Emitter {
    constructor(container, options) {
        super()
        this.$container = container;
        this._opt = Object.assign({}, DEFAULT_PLAYER_OPTIONS, options)


        if (this._opt.useWCS) {
            this._opt.useWCS = supportWCS();
        }

        if(!this._opt.forceNoOffscreen){
            this._opt.forceNoOffscreen = supportOffscreen();
        }


        property(this);
        events(this);
        this._loading = true;
        this._hasLoaded = false;

        this.debug = new Debug(this);
        this.events = new Events(this);
        this.video = new Video(this);
        this.stream = new Stream(this);
        this.audio = new Audio(this);
        this.recorder = new Recorder(this);
        this.demux = new Demux(this);
        this.decoderWorker = new DecoderWorker(this);

        this.debug.log('options', this._opt);
    }


    play() {
        return new Promise((resolve, reject) => {

        })
    }


    destroy() {
        if (this.events) {
            this.events.destroy();
            this.events = null;
        }
        if (this.decoderWorker) {
            this.decoderWorker.destroy();
            this.decoderWorker = null;
        }
        if (this.video) {
            this.video.destroy();
            this.video = null;
        }
        if (this.audio) {
            this.audio.destroy();
            this.audio = null;
        }

        if (this.stream) {
            this.stream.destroy();
            this.stream = null;
        }

        if (this.recorder) {
            this.recorder.destroy();
            this.recorder = null;
        }

        // 其他没法解耦的，通过 destroy 方式
        this.emit('destroy');
        // 接触所有绑定事件
        this.off();

        this.debug.log('play', 'destroy end');
    }

}
