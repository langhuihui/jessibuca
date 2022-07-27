import EventEmitter from 'eventemitter3'
import {VideoDecoderType, VideoCodecInputInfo, VideoPacket} from './types'



export class VideoDecoder extends EventEmitter {


    constructor(vdtype: VideoDecoderType) {

        super();

    };


    setVideoCodec(vCodeInputInfo: VideoCodecInputInfo): void {


    };

    decodeVideo(vPacket: VideoPacket): void {

    }

};
