import EventEmitter from 'eventemitter3';
import {DecoderState, VideoDecoderConfig, VideoPacket, VideoDecoderInterface} from './types';
import Module from '../wasm/out/decodervideo';



export class VideoSoftDecoder extends EventEmitter implements VideoDecoderInterface {

    decoderState: DecoderState;

    constructor(useSIMD: boolean) {

        super();
        this.decoderState = 'configured';

    };

    state(): DecoderState {

        return this.decoderState;

    }


    configure(config: VideoDecoderConfig): void {


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
