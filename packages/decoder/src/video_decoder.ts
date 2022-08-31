import EventEmitter from 'eventemitter3';
import {DecoderState, VideoDecoderType, VideoDecoderConfig, VideoPacket, VideoDecoderEvent, VideoDecoderInterface, VideoCodecInfo, JVideoFrame, ErrorInfo} from './types'
import { VideoDecoderSoft } from './video_decoder_soft';
import { VideoDecoderSoftSIMD } from './video_decoder_soft_simd';
import { VideoDecoderHard } from './video_decoder_hard';

export class VideoDecoder extends EventEmitter implements VideoDecoderInterface {

    decoder: VideoDecoderSoft | VideoDecoderSoftSIMD | VideoDecoderHard;

    constructor(vdtype: VideoDecoderType) {

        super();

        if (vdtype === 'soft') {

            this.decoder = new VideoDecoderSoft();

        } else if (vdtype === 'soft-simd') {

            this.decoder = new VideoDecoderSoftSIMD();

        } else if (vdtype === 'hard') {

            this.decoder = new VideoDecoderHard();

        } else if (vdtype === 'auto') {

            this.decoder = new VideoDecoderSoft();

        } else {

            throw new Error(`video type [${vdtype}] not support`);
        }

        this.decoder.on(VideoDecoderEvent.VideoCodecInfo, (codecinfo: VideoCodecInfo) => {

            this.emit(VideoDecoderEvent.VideoCodecInfo, codecinfo);
        })

        this.decoder.on(VideoDecoderEvent.VideoFrame, (videoFrame: JVideoFrame) => {

            this.emit(VideoDecoderEvent.VideoFrame, videoFrame);
        })

        this.decoder.on(VideoDecoderEvent.Error, (error: ErrorInfo) => {

            this.emit(VideoDecoderEvent.Error, error);
        })

    };

    initialize(): Promise<void>{ 

        return this.decoder.initialize();
    }


    state(): DecoderState {

        return this.decoder.state();
    }

    configure(config: VideoDecoderConfig): void {

        this.decoder.configure(config);
    }

    decode(packet: VideoPacket): void {

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
