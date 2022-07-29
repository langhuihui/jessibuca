import EventEmitter from 'eventemitter3';
import {DecoderState, VideoDecoderConfig, VideoPacket, VideoDecoderInterface} from './types';
import VideoDecoderModule from '../wasm/out/decodervideo';


let moduleOverrides: any = {
    print: (s:any) => {
        console.log(s);
    },
    printErr: (e:any) => {
        console.error(e);
    }
};


export class VideoSoftDecoder extends EventEmitter implements VideoDecoderInterface {

    decoderState: DecoderState;
    decoder: any;
    config: undefined | VideoDecoderConfig;

    constructor(useSIMD: boolean) {

        super();
        this.decoderState = 'unconfigured';

    };

    state(): DecoderState {

        return this.decoderState;

    }

    configure(config: VideoDecoderConfig): void {

        if (this.decoderState !== 'unconfigured') {

            return;
        }

        this.config = config;

        VideoDecoderModule(moduleOverrides).then((Module: any) => {
            //console.log("load HelloWorldModule success.", Module);

            this.decoder = new Module.VideoDecoder(this);

        }).catch((error:any) => {

            console.log("load HelloWorldModule failed with error:", error);
        });

        this.decoderState = 'configured';


    }
    decode(packet: VideoPacket): void {

    }

    flush(): void {

    }

    reset(): void {

    }

    close(): void {

    }

};
