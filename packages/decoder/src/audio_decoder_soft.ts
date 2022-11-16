import {
  AudioDecoderConfig,
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
    this.decoder.setCodec(this.config.codec, this.config.extraData);
  }
  @Includes("configured")
  decode(packet: EncodedAudioChunkInit): void {
    this.decoder.decode(packet.data, packet.timestamp);
  }

  flush(): void {}
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

    for (let i = 0; i < this.channels; i++) {
      let fp = this.module.HEAPU32[(pcmDataArray >> 2) + i] >> 2;
      pcmDatas.push(
        Float32Array.of(...this.module.HEAPF32.subarray(fp, fp + samples))
      );
    }

    let aFrame: AudioFrame = {
      datas: pcmDatas,
      sampleNum: samples,
      channles: this.channels,
      pts: pts,
    };

    this.emit(AudioDecoderEvent.AudioFrame, aFrame);
  }

  errorInfo(errormsg: string): void {
    let err: ErrorInfo = {
      errMsg: errormsg,
    };

    this.emit(AudioDecoderEvent.Error, err);
  }
}
