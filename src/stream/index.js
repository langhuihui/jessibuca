import FetchLoader from "./fetchLoader";
import WebsocketLoader from "./websocketLoader";
import WebTransportLoader from "./webTransportLoader";
import { PLAYER_PLAY_PROTOCOL } from "../constant";

export default class Stream {
    constructor(player) {
        const Loader = Stream.getLoaderFactory(player._opt.protocol);
        return new Loader(player);
    }

    static getLoaderFactory(protocol) {
        switch (protocol) {
            case PLAYER_PLAY_PROTOCOL.fetch:
                return FetchLoader;
            case PLAYER_PLAY_PROTOCOL.websocket:
                return WebsocketLoader;
            case PLAYER_PLAY_PROTOCOL.webtransport:
                return WebTransportLoader;
            default:
                throw new Error(`unsupported protocol: ${protocol}`);
        }
    }
}
