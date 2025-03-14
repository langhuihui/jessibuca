import {
  AudioDecoderInterface,
  AudioDecoderEvent,
} from "./types";
import { ChangeState, FSM, Includes } from "afsm";

export class AudioDecoderHard extends FSM implements AudioDecoderInterface {
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
  @ChangeState("initialized", "configured", { sync: true })
  configure(config: AudioDecoderConfig): void {
    this.config = config;
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
  @ChangeState([], FSM.INIT, { sync: true })
  reset(): void {
    this.decoder.reset();
  }
  @ChangeState([], "closed", { ignoreError: true, sync: true })
  close(): void {
    if (this.decoder.state !== "closed") this.decoder.close();
  }
}
