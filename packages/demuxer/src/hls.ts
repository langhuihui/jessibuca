import { BaseDemuxer, DemuxMode, Source } from "./base";
import { HlsOption } from "./hls/config";
import HlsLoader from './hls/index';

export class HLSDemuxer extends BaseDemuxer {
  private hlsLoader: HlsLoader;

  constructor(url: string, cfg: HlsOption) {
    super(undefined, DemuxMode.PUSH, 'annexb');
    this.hlsLoader = new HlsLoader(cfg);
    this.hlsLoader.loadSource(url);
  }

  async pull(): Promise<void> {
    throw new Error("Method not implemented.");
  }

  *demux(): Generator<number | Uint8Array, void, Uint8Array> {
    // Implementation will be added based on buffer service data
    throw new Error("Method not implemented.");
  }
}