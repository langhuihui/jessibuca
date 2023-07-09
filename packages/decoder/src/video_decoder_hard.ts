import {
  VideoDecoderInterface,
  VideoDecoderEvent,
} from "./types";
import { ChangeState, FSM, Includes } from "afsm";
export class VideoDecoderHard extends FSM implements VideoDecoderInterface {
  decoder!: VideoDecoder;
  config?: VideoDecoderConfig;
  @ChangeState([FSM.INIT, "closed"], "initialized")
  async initialize() {
    this.decoder = new VideoDecoder({
      output: (frame) => {
        this.emit(VideoDecoderEvent.VideoFrame, frame);
      },
      error: (err) => {
        this.emit(VideoDecoderEvent.Error, err);
        this.close();
      },
    });
  }
  @ChangeState("initialized", "configured")
  configure(config: VideoDecoderConfig): void {
    this.config = config;
    console.log("configure", config);
    this.decoder.configure({
      ...config,
      codec: config.codec == "hevc" ? 'hvc1.1.6.L0.12.34.56.78.9A.BC' : 'avc1.420028'
    });
  }
  @Includes("configured")
  decode(packet: EncodedVideoChunkInit): void {
    if (this.decoder.state === "configured")
      this.decoder.decode(new EncodedVideoChunk(packet));
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
