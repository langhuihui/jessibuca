import EventEmitter from "eventemitter3";
import {
  AudioDecoderInterface,
  AudioDecoderEvent,
} from "./types";
import { ChangeState, FSM, Includes } from "afsm";

export class AudioDecoderHard
  extends EventEmitter
  implements AudioDecoderInterface
{
  decoder!: AudioDecoder;
  config?: AudioDecoderConfig;
  @ChangeState([FSM.INIT, "closed"], "initialized")
  async initialize() {
    this.decoder = new AudioDecoder({
      output: (frame) => {
        this.emit(AudioDecoderEvent.AudioFrame, frame);
      },
      error: (err) => {
        this.emit(AudioDecoderEvent.Error, err);
        this.close();
      },
    });
  }
  @ChangeState("initialized", "configured")
  configure(config: AudioDecoderConfig): void {
    this.config = config;
    console.log("configure", config);
    this.decoder.configure(config);
  }
  @Includes("configured")
  decode(packet: EncodedAudioChunkInit): void {
    if (this.decoder.state === "configured")
      this.decoder.decode(new EncodedAudioChunk(packet));
  }
  flush(): void {
    this.decoder.flush();
  }
  @ChangeState([], FSM.INIT)
  reset(): void {
    this.decoder.reset();
  }
  @ChangeState([], "closed", { ignoreError: true })
  close(): void {
    this.decoder.close();
  }
}
