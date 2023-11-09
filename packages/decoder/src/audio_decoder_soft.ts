import {
  AudioDecoderInterface,
  AudioCodecInfo,
  AudioDecoderEvent,
  AudioFrame,
  ErrorInfo,
} from "./types";
import CreateModule from "../wasm/types/audiodec";
import { ChangeState, FSM, Includes } from "afsm";

export class AudioDecoderSoft extends FSM implements AudioDecoderInterface {
  decoder: any;
  config?: AudioDecoderConfig;
  module?: any;

  sampleRate = 0;
  channels = 0;

  @ChangeState(FSM.INIT, "initialized")
  initialize(): Promise<void> {
    return new Promise((resolve) => {
      const opts: any = {};
      opts.print = (text: string) => console.log(text);
      opts.printErr = (text: string) => console.log(`[JS] ERROR: ${text}`);
      opts.onAbort = () => console.log("[JS] FATAL: WASM ABORTED");
      opts.postRun = (m: any) => {
        this.module = m;
        this.decoder = new this.module.AudioDecoder(this);
        resolve();
      };
      console.log(`audio soft decoder initialize call`);
      CreateModule(opts);
    });
  }
  @ChangeState("initialized", "configured")
  configure(config: AudioDecoderConfig): void {
    this.config = config;
    this.decoder.setCodec(this.config.codec, this.config.description ?? '');
  }
  @Includes("configured")
  decode(packet: EncodedAudioChunkInit): void {
    this.decoder.decode(packet.data, packet.timestamp);
  }

  flush(): void { }
  @ChangeState("configured", "initialized")
  reset(): void {
    this.config = undefined;
    if (this.decoder) {
      this.decoder.clear();
    }
  }
  @ChangeState([], "closed")
  close(): void {
    this.removeAllListeners();
    if (this.decoder) {
      this.decoder.clear();
      this.decoder.delete();
    }
  }

  // wasm callback function
  audioInfo(sampleRate: number, channels: number): void {
    this.sampleRate = sampleRate;
    this.channels = channels;

    let audioCodeInfo: AudioCodecInfo = {
      sampleRate,
      channels,
      depth: 16,
    };

    this.emit(AudioDecoderEvent.AudioCodecInfo, audioCodeInfo);
  }

  pcmData(pcmDataArray: number, samples: number, pts: number): void {
    if (!this.module) {
      return;
    }

    let pcmDatas: Float32Array[] = [];
    let size = 0;
    let offset = 0;
    for (let i = 0; i < this.channels; i++) {
      let fp = this.module.HEAPU32[(pcmDataArray >> 2) + i] >> 2;
      const data = this.module.HEAPF32.subarray(fp, fp + samples);
      pcmDatas.push(data);
      size += data.length;
    }
    const data = new Float32Array(size);
  
    this.emit(AudioDecoderEvent.AudioFrame, new AudioData({
      format: "f32-planar",
      sampleRate: this.sampleRate,
      numberOfChannels: this.channels,
      timestamp: pts,
      numberOfFrames: samples,
      data: pcmDatas.reduce((prev, curr) => {
        prev.subarray(offset).set(curr);
        offset += curr.length;
        return prev;
      }, data),
    }));
  }

  errorInfo(errormsg: string): void {
    let err: ErrorInfo = {
      errMsg: errormsg,
    };

    this.emit(AudioDecoderEvent.Error, err);
  }
}
