import EventEmitter from 'eventemitter3';
import {DecoderState, AudioDecoderType, AudioDecoderConfig, AudioPacket, AudioDecoderEvent, AudioDecoderInterface, AudioCodecInfo, AudioFrame, ErrorInfo} from './types'
import { AudioSoftDecoder } from './audiosoftdecoder';

export class AudioDecoder extends EventEmitter implements AudioDecoderInterface {

    decoder: AudioDecoderInterface;

    constructor(adtype: AudioDecoderType) {

        super();

        if (adtype === 'software-decoder') {

            this.decoder = new AudioSoftDecoder();

        } else if (adtype === 'auto') {

            this.decoder = new AudioSoftDecoder();

        } else {

            throw new Error(`Audio type [${adtype}] not support`);
        }

        this.on(AudioDecoderEvent.AudioCodecInfo, (codecinfo: AudioCodecInfo) => {

            this.emit(AudioDecoderEvent.AudioCodecInfo, codecinfo);
        })

        this.on(AudioDecoderEvent.AudioFrame, (AudioFrame: AudioFrame) => {

            this.emit(AudioDecoderEvent.AudioFrame, AudioFrame);
        })

        this.on(AudioDecoderEvent.Error, (error: ErrorInfo) => {

            this.emit(AudioDecoderEvent.Error, error);
        })

    };

    
    initialize(): Promise<void>{ 

        return this.decoder.initialize();
    }


    state(): DecoderState {

        return this.decoder.state();
    }

    configure(config: AudioDecoderConfig): void {

        this.decoder.configure(config);
    }

    decode(packet: AudioPacket): void {

        this.decoder.decode(packet);

    }

    flush(): void {

        this.decoder.flush();

    }

    reset(): void {

        this.decoder.reset();
    }

    close(): void {

        this.decoder.close();
        this.removeAllListeners();
    }

};
