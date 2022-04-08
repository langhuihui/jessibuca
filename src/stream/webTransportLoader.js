import Emitter from "../utils/emitter";
import {calculationRate, now} from "../utils";
import {EVENTS} from "../constant";
import OPut from 'oput'

export default class WebTransportLoader extends Emitter {
    constructor(player) {
        super();
        this.player = player;
        this.transport = null;
        this.wtUrl = null;

        //
        this.streamRate = calculationRate(rate => {
            player.emit(EVENTS.kBps, (rate / 1024).toFixed(2));
        });
        player.debug.log('WebTransportLoader', 'init');
    }

    destroy() {
        if (this.transport) {
            this.transport = null;
        }
    }

    async _createWebTransport() {
        const player = this.player;
        const {
            debug,
            events: {proxy},
            demux,
        } = player;

        this.transport = new WebTransport(this.wtUrl);
        await this.transport.ready
        return this.transport.createBidirectionalStream().then((stream)=>{
            stream.readable.pipeTo(new WritableStream(demux.input));
        })
    }

    fetchStream(url) {
        this.player._times.streamStart = now();
        this.wtUrl = url;
        this._createWebTransport();
    }
}
