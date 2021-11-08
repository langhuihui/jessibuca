import FetchLoader from "./fetchLoader";
import WebsocketLoader from "./websocketLoader";
import {PLAYER_PLAY_PROTOCOL} from "../constant";

export default class Stream {
    constructor(player) {
        const Loader = Stream.getLoaderFactory(player._opt.protocol);
        return new Loader(player);
    }

    static getLoaderFactory(protocol) {
        if (protocol === PLAYER_PLAY_PROTOCOL.fetch) {
            return FetchLoader;
        } else if (protocol === PLAYER_PLAY_PROTOCOL.websocket) {
            return WebsocketLoader;
        }
    }
}
