import Emitter from "../utils/emitter";
import {EVENTS, EVENTS_ERROR, WEBSOCKET_STATUS} from "../constant";
import {calculationRate} from "../utils";

export default class WebsocketLoader extends Emitter {
    constructor(player) {
        super();
        this.player = player;
        this.socket = null;
        this.socketStatus = WEBSOCKET_STATUS.notConnect;
        this.wsUrl = null;
        //
        this.streamRate = calculationRate(rate => {
            player.emit(EVENTS.kBps, (rate / 1024).toFixed(2));
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
            this.emit(EVENTS.streamSuccess);
            debug.log('websocketLoader', 'socket open');
            this.socketStatus = WEBSOCKET_STATUS.open;
        });

        proxy(this.socket, 'message', event => {
            this.streamRate && this.streamRate(event.data.byteLength);
            this._handleMessage(event.data);
        });


        proxy(this.socket, 'close', () => {
            debug.log('websocketLoader', 'socket close');
            this.emit(EVENTS.streamEnd);
            this.socketStatus = WEBSOCKET_STATUS.close;
        });

        proxy(this.socket, 'error', error => {
            debug.log('websocketLoader', 'socket error');
            this.emit(EVENTS_ERROR.websocketError, error);
            this.player.emit(EVENTS.error, EVENTS_ERROR.websocketError);
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
        this.streamRate = null;
        this.off();
        this.player.debug.log('websocketLoader', 'destroy');
    }
}
