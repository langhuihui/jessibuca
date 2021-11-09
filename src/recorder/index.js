import RecordRTCLoader from "./recordRTCLoader";

export default class Recorder {
    constructor(player) {
        const Loader = Recorder.getLoaderFactory();
        return new Loader(player);
    }

    static getLoaderFactory() {
        return RecordRTCLoader;
    }
}
