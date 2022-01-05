import Emitter from "../utils/emitter";
import {EVENTS, EVENTS_ERROR, JESSIBUCA_EVENTS} from "../constant";
import {calculationRate} from "../utils";

export default class FetchLoader extends Emitter {
    constructor(player) {
        super();
        this.player = player;
        this.playing = false;

        this.abortController = new AbortController();
        //
        this.streamRate = calculationRate(rate => {
            player.emit(EVENTS.kBps, (rate / 1024).toFixed(2));
        });
        player.debug.log('FetchStream', 'init');
    }

    fetchStream(url) {
        const {demux} = this.player;
        fetch(url, {signal: this.abortController.signal}).then((res) => {
            const reader = res.body.getReader();
            this.emit(EVENTS.streamSuccess);
            const fetchNext = () => {
                reader.read().then(({done, value}) => {
                        if (done) {
                            demux.close();
                        } else {
                            this.streamRate && this.streamRate(value.byteLength);
                            demux.dispatch(value);
                            fetchNext();
                        }
                    }
                ).catch((e) => {
                    demux.close();
                    // 这边会报用户 aborted a request 错误。
                    this.emit(EVENTS_ERROR.fetchError, e);
                    this.player.emit(EVENTS.error, EVENTS_ERROR.fetchError);
                    this.abort();
                })
            }
            fetchNext();
        }).catch((e) => {
            this.abort();
            this.emit(EVENTS_ERROR.fetchError, e)
            this.player.emit(EVENTS.error, EVENTS_ERROR.fetchError);
        })
    }

    abort() {
        if (this.abortController) {
            this.abortController.abort();
            this.abortController = null
        }
    }


    destroy() {
        this.abort()
        this.off();
        this.streamRate = null;
        this.player.debug.log('FetchStream', 'destroy');
    }
}
