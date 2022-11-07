import EventEmitter from "eventemitter3";
import {
  AudioDecoderConfig,
  AudioDecoderInterface,
  AudioPacket,
  DecoderState,
} from "./types";

export class AudioDecoderHard
  extends EventEmitter
  implements AudioDecoderInterface
{
  state(): DecoderState {
    throw new Error("Method not implemented.");
  }
  initialize(): Promise<void> {
    throw new Error("Method not implemented.");
  }
  configure(config: AudioDecoderConfig): void {
    throw new Error("Method not implemented.");
  }
  decode(packet: AudioPacket): void {
    throw new Error("Method not implemented.");
  }
  flush(): void {
    throw new Error("Method not implemented.");
  }
  reset(): void {
    throw new Error("Method not implemented.");
  }
  close(): void {
    throw new Error("Method not implemented.");
  }
}
