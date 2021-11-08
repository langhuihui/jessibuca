import AudioContextLoader from "./audioContextLoader";
import AudioLoader from "./audioLoader";

export default class Audio {
    constructor(player) {
        const Loader = Audio.getLoaderFactory();
        return new Loader(player);
    }

    static getLoaderFactory() {
        return AudioContextLoader;
    }
}
