import EventEmitter from 'eventemitter3';
import {DecoderState, AudioDecoderConfig, AudioPacket, AudioDecoderInterface} from './types'


export class AudioSoftDecoder extends EventEmitter implements AudioDecoderInterface {

    decoderState: DecoderState;

    constructor() {

        super();
        this.decoderState = 'configured';

    };

    state(): DecoderState {

        return this.decoderState;

    }


    configure(config: AudioDecoderConfig): void {


    }
    decode(packet: AudioPacket): void {

    }

    flush(): void {

    }

    reset(): void {

    }

    close(): void {

    }

};
