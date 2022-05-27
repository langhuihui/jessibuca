import Emitter from "../utils/emitter";
import {calculationRate, now} from "../utils";
import {EVENTS} from "../constant";

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
            // console.log(this.transport.state)
            // this.transport.close().then(()=>{
            //
            // }).catch((e)=>{
            //     console.error(e);
            // });

            this.transport.closed.then(() => {
                // console.log(`The HTTP/3 connection to ${url} closed gracefully.`);
                this.player.debug.log('WebTransportLoader', 'destroy');
                this.transport = null;
            }).catch((error) => {
                console.error('The HTTP/3 connection to ${url} closed due to ${error}.');
            });
        }
    }

    _createWebTransport() {
        const player = this.player;
        const {
            debug,
            events: {proxy},
            demux,
        } = player;

        this.transport = new WebTransport(this.wtUrl);
        this.transport.ready.then(() => {
            this.transport.createBidirectionalStream().then((stream) => {
                stream.readable.pipeTo(new WritableStream(demux.input));
            })
        })

    }

    fetchStream(url) {
        this.player._times.streamStart = now();
        this.wtUrl = url;
        this._createWebTransport();
    }
}
