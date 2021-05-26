import {CMD_TYPE} from "../constant";

export default (jessibuca) => {
    const decoderWorker = new Worker(jessibuca._opt.decoder);


    decoderWorker.onmessage = (event) => {
        const msg = event.data;
        switch (msg.cmd) {
            case CMD_TYPE.init:

        }
    }

    jessibuca._decoderWorker = decoderWorker;
}
