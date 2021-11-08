import CanvasVideoLoader from './canvasLoader';
import VideoLoader from "./videoLoader";

export default class Video {
    constructor(player) {
        const Loader = Video.getLoaderFactory();
        return new Loader(player);
    }

    static getLoaderFactory() {
        return CanvasVideoLoader;
    }
}
