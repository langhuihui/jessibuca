import EventEmitter from 'eventemitter3';
import {DecoderState, VideoDecoderType, VideoDecoderConfig, VideoPacket, VideoDecoderEvent, VideoDecoderInterface, VideoCodecInfo, VideoFrame, ErrorInfo} from './types'
import { VideoSoftDecoder } from './videosoftdecoder';
import { VideoHardDecoder } from './videoharddecoder';
import { VideoSoftDecoderSIMD } from './videosoftdecodersimd';

export class VideoDecoder extends EventEmitter implements VideoDecoderInterface {

    decoder: VideoDecoderInterface;

    constructor(vdtype: VideoDecoderType) {

        super();

        if (vdtype === 'software-decoder') {

            this.decoder = new VideoSoftDecoder();

        } else if (vdtype === 'software-simd-decoder') {

            this.decoder = new VideoSoftDecoderSIMD();

        } else if (vdtype === 'hardware-decoder') {

            this.decoder = new VideoHardDecoder();

        } else if (vdtype === 'auto') {

            this.decoder = new VideoSoftDecoder();

        } else {

            throw new Error(`video type [${vdtype}] not support`);
        }

        this.on(VideoDecoderEvent.VideoCodecInfo, (codecinfo: VideoCodecInfo) => {

            this.emit(VideoDecoderEvent.VideoCodecInfo, codecinfo);
        })

        this.on(VideoDecoderEvent.VideoFrame, (videoFrame: VideoFrame) => {

            this.emit(VideoDecoderEvent.VideoFrame, videoFrame);
        })

        this.on(VideoDecoderEvent.Error, (error: ErrorInfo) => {

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
