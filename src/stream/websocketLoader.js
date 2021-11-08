import Emitter from "../utils/emitter";
import {EVEMTS, WEBSOCKET_STATUS} from "../constant";
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
            player.emit(EVEMTS.streamRate, rate);
        });
    }

    _createWebSocket() {

    }

    _handleMessage() {

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
