import { VideoDecoderSoftBase } from './video_decoder_soft_base';
import CreateModule, { WASMModule } from '../wasm/types/videodec';


export class VideoDecoderSoft extends VideoDecoderSoftBase {

    constructor()  {

        super(CreateModule);
    };

};
