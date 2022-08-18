import EventEmitter from 'eventemitter3';
import {DecoderState, AudioDecoderType, AudioDecoderConfig, AudioPacket, AudioDecoderEvent, AudioDecoderInterface, AudioCodecInfo, AudioFrame, ErrorInfo} from './types'
import { AudioDecoderSoft } from './audio_decoder_soft';

export class AudioDecoder extends EventEmitter implements AudioDecoderInterface {

    decoder: AudioDecoderSoft;

    constructor(adtype: AudioDecoderType) {

        super();

        if (adtype === 'soft') {

            this.decoder = new AudioDecoderSoft();

        } else if (adtype === 'auto') {

            this.decoder = new AudioDecoderSoft();

        } else {

            throw new Error(`Audio type [${adtype}] not support`);
        }

        this.decoder.on(AudioDecoderEvent.AudioCodecInfo, (codecinfo: AudioCodecInfo) => {

            this.emit(AudioDecoderEvent.AudioCodecInfo, codecinfo);
        })

        this.decoder.on(AudioDecoderEvent.AudioFrame, (AudioFrame: AudioFrame) => {

            this.emit(AudioDecoderEvent.AudioFrame, AudioFrame);
        })

        this.decoder.on(AudioDecoderEvent.Error, (error: ErrorInfo) => {

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
