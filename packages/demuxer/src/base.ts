import { EventEmitter } from "eventemitter3";
import Oput from "oput";
export const enum DemuxEvent {
  AUDIO_ENCODER_CONFIG_CHANGED = "audio-encoder-config-changed",
  VIDEO_ENCODER_CONFIG_CHANGED = "video-encoder-config-changed",
}
export const enum DemuxMode {
  PULL,
  PUSH
}
export interface Source {
  oput?: Oput;
  read<T extends number | Uint8Array>(need: T): Promise<Uint8Array>;
}
export abstract class BaseDemuxer extends EventEmitter<{
  [DemuxEvent.AUDIO_ENCODER_CONFIG_CHANGED]: [AudioDecoderConfig];
  [DemuxEvent.VIDEO_ENCODER_CONFIG_CHANGED]: [VideoDecoderConfig];
}> {
  constructor(
    public source?: Source,
    public mode: DemuxMode = DemuxMode.PULL
  ) {
    super();
    console.log("Demuxer Created:", Object.getPrototypeOf(this).constructor.name);
    if (source) {
      if (mode == DemuxMode.PULL) {
        this.startPull(source);
      } else {
        source.oput = new Oput(this.demux());
      }
    }
  }
  audioReadable?: ReadableStream<EncodedAudioChunkInit>;
  videoReadable?: ReadableStream<EncodedVideoChunkInit>;
  audioDecoderConfig?: AudioDecoderConfig;
  videoDecoderConfig?: VideoDecoderConfig;
  abstract pull(): Promise<void>;
  startPull(source: Source) {
    this.mode = DemuxMode.PULL;
    this.source = source;
    this.audioReadable = new ReadableStream({
      pull: async (controller) => controller.enqueue(await this.pullAudio()),
    });
    this.videoReadable = new ReadableStream({
      pull: async (controller) => controller.enqueue(await this.pullVideo()),
    });
  }
  abstract demux(): Generator<number | Uint8Array, void, Uint8Array>;
  gotAudio?: (data: EncodedAudioChunkInit) => void;
  gotVideo?: (data: EncodedVideoChunkInit) => void;
  pullAudio(): Promise<EncodedAudioChunkInit> {
    return new Promise<EncodedAudioChunkInit>((resolve, reject) => {
      this.gotAudio = resolve;
      //只调用一次pull
      if (!!this.gotVideo) this.pull().catch(reject);
    });
  }
  pullVideo(): Promise<EncodedVideoChunkInit> {
    return new Promise<EncodedVideoChunkInit>((resolve, reject) => {
      this.gotVideo = resolve;
      //只调用一次pull
      if (!!this.gotAudio) this.pull().catch(reject);
    });
  }
}
