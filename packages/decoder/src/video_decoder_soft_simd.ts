import { VideoDecoderSoftBase } from './video_decoder_soft_base';
import CreateModule from '../wasm/types/videodec_simd';
export class VideoDecoderSoftSIMD extends VideoDecoderSoftBase {
    constructor()  {
        super(CreateModule);
    };
};
