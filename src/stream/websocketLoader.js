import Emitter from "../utils/emitter";
import {EVENTS, WEBSOCKET_STATUS} from "../constant";
import {calculationRate} from "../utils";

export default class WebsocketLoader extends Emitter {
    constructor(player) {
        super();
        this.player = player;
        this.socket = null;
        this.socketStatus = WEBSOCKET_STATUS.notConnect;
        this.playing = false;
        this.wsUrl = null;
        //
        this.streamRate = calculationRate(rate => {
            player.emit(EVENTS.streamRate, rate);
        });
    }

    _createWebSocket() {
        const player = this.player;
        const {
            debug,
            events: {proxy},
            demux,
        } = player;

        this.socket = new WebSocket(this.wsUrl);
        this.socket.binaryType = 'arraybuffer';
        proxy(this.socket, 'open', () => {
            debug.log('websocketLoader', 'socket open -> send login');
            this.socketStatus = WEBSOCKET_STATUS.open;
        });

        proxy(this.socket, 'message', event => {
            const message = new Uint8Array(event.data);
            this.streamRate(message.byteLength);
            this._handleMessage(message);
        });


        proxy(this.socket, 'close', () => {
            player.emit(EVENTS.streamEnd);
            this.socketStatus = WEBSOCKET_STATUS.close;
        });

        proxy(this.socket, 'error', error => {
            player.emit(EVENTS.streamError, error);
            this.socketStatus = WEBSOCKET_STATUS.error;
            demux.close();
            debug.log('websocketLoader', `socket error:`, error);
        });
    }

    //
    _handleMessage(message) {
        const {demux} = this.player;
        demux.dispatch(message);
    }


    fetchStream(url) {
        this.wsUrl = url;
        this._createWebSocket();
    }


    destroy() {
        if (this.socket) {
            this.socket.close();
            this.socket = null;
        }
        this.socketStatus = WEBSOCKET_STATUS.notConnect;
        this.playing = false;
        this.off();
        this.player.debug.log('websocketLoader', 'destroy');

    }
}
