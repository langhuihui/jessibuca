import { FlvDemuxer } from "./flv";
import { EventEmitter } from 'eventemitter3';
export interface DemuxerReader {
  type: 'flv' | 'mp4' | 'hls' | 'm7s' | 'unknown';
  read<T extends number | Uint8Array>(need: T): Promise<Uint8Array>;
}
export class Demuxer extends EventEmitter {
  audioReadable: ReadableStream<EncodedAudioChunkInit>;
  videoReadable: ReadableStream<EncodedVideoChunkInit>;
  audioEncoderConfig?: AudioEncoderConfig;
  videoEncoderConfig?: VideoEncoderConfig;
  static AUDIO_ENCODER_CONFIG_CHANGED = 'audio-encoder-config-changed';
  static VIDEO_ENCODER_CONFIG_CHANGED = 'video-encoder-config-changed';
  constructor(reader: DemuxerReader) {
    super();
    let pull: () => Promise<void>;
    let gotAudio: (data: EncodedAudioChunkInit) => void;
    let gotVideo: (data: EncodedVideoChunkInit) => void;
    const pullAudio = () => new Promise<EncodedAudioChunkInit>((resolve, reject) => {
      gotAudio = resolve;
      if (!!gotVideo) pull().catch(reject);
    });
    const pullVideo = () => new Promise<EncodedVideoChunkInit>((resolve, reject) => {
      gotVideo = resolve;
      if (!!gotAudio) pull().catch(reject);
    });
    switch (reader.type) {
      case 'flv':
        const r = new FlvDemuxer(reader).getReader();
        pull = (): Promise<void> => r.read().then(({ value, done }) => {
          if (value) {
            switch (value.type) {
              case 8:
                if (!this.audioEncoderConfig) {
                  this.audioEncoderConfig = {
                    codec: { 10: 'aac', 7: 'pcma', 8: 'pcmu' }[value.data[0] >> 4] || 'unknown',
                  };
                  //TODO: parse audio config
                  if (this.audioEncoderConfig.codec == 'aac') {
                  }
                }
                if (this.audioEncoderConfig.codec == 'aac') {
                  if (value.data[1] == 0x00) {
                    this.emit(Demuxer.AUDIO_ENCODER_CONFIG_CHANGED, value.data.subarray(2));
                    return pull();
                  }
                }
                return gotAudio({
                  type: 'key',
                  data: value.data,
                  timestamp: value.timestamp,
                  duration: 0
                });
              case 9:
                if (!this.videoEncoderConfig) {
                  this.videoEncoderConfig = {
                    codec: { 7: 'h264', 12: 'h265' }[value.data[0] & 0xf] || 'unknown',
                  };
                  //TODO: parse video config
                }
                if (value.data[1] == 0x00) {
                  this.emit(Demuxer.VIDEO_ENCODER_CONFIG_CHANGED, value.data.subarray(2));
                  return pull();
                }
                return gotVideo({
                  type: value.data[0] >> 4 == 1 ? 'key' : 'delta',
                  data: value.data,
                  timestamp: value.timestamp,
                  duration: 0
                });
              default:
                return pull();
            }
          }
        });
    }
    this.audioReadable = new ReadableStream({
      pull: async controller => controller.enqueue(await pullAudio())
    });
    this.videoReadable = new ReadableStream({
      pull: async controller => controller.enqueue(await pullVideo())
    });
  }
}