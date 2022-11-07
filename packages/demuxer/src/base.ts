import { EventEmitter } from "eventemitter3";
export const enum DemuxEvent {
  AUDIO_ENCODER_CONFIG_CHANGED = "audio-encoder-config-changed",
  VIDEO_ENCODER_CONFIG_CHANGED = "video-encoder-config-changed",
}
export abstract class BaseDemuxer extends EventEmitter {
  constructor(
    public source: {
      read<T extends number | Uint8Array>(need: T): Promise<Uint8Array>;
    }
  ) {
    super();
    this.demux();
  }
  audioReadable: ReadableStream<EncodedAudioChunkInit> = new ReadableStream({
    pull: async (controller) => controller.enqueue(await this.pullAudio()),
  });
  videoReadable: ReadableStream<EncodedVideoChunkInit> = new ReadableStream({
    pull: async (controller) => controller.enqueue(await this.pullVideo()),
  });
  audioEncoderConfig?: AudioEncoderConfig;
  videoEncoderConfig?: VideoEncoderConfig;
  abstract demux(): void;
  abstract pull(): Promise<void>;
  gotAudio?: (data: EncodedAudioChunkInit) => void;
  gotVideo?: (data: EncodedVideoChunkInit) => void;
  pullAudio(): Promise<EncodedAudioChunkInit> {
    return new Promise<EncodedAudioChunkInit>((resolve, reject) => {
      this.gotAudio = resolve;
      if (!!this.gotAudio) this.pull().catch(reject);
    });
  }
  pullVideo(): Promise<EncodedVideoChunkInit> {
    return new Promise<EncodedVideoChunkInit>((resolve, reject) => {
      this.gotVideo = resolve;
      if (!!this.gotVideo) this.pull().catch(reject);
    });
  }
}
