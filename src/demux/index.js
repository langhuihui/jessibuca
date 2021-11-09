import FlvLoader from "./flvLoader";
import M7sLoader from "./m7sLoader";
import {DEMUX_TYPE} from "../constant";

export default class Demux {
    constructor(player) {
        const Loader = Demux.getLoaderFactory(player._opt.demuxType);
        return new Loader(player);
    }

    static getLoaderFactory(type) {
        if (type === DEMUX_TYPE.m7s) {
            return M7sLoader;
        } else if (type === DEMUX_TYPE.flv) {
            return FlvLoader;
        }
    }
}
