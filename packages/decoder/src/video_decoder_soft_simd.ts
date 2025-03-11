import { VideoDecoderSoftBase } from './video_decoder_soft_base';
import CreateModule from '../wasm/types/videodec_simd';
export class VideoDecoderSoftSIMD extends VideoDecoderSoftBase {
  constructor(opt?: { workerMode?: boolean, canvas?: HTMLCanvasElement,wasmPath?: string, yuvMode?:boolean; }) {
    super(CreateModule, opt?.wasmPath ? fetch(opt.wasmPath).then(res => res.arrayBuffer()): void 0, opt?.workerMode, opt?.canvas, opt?.yuvMode);
  };
};
