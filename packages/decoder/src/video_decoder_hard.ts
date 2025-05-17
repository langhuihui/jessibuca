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
        this.close();
        this.emit(VideoDecoderEvent.Error, err);
      },
    });
  }
  @ChangeState("initialized", "configured", { sync: true })
  configure(config: VideoDecoderConfig): void {
    this.config = config;
    if (!config.description && config.codec !== "av1") {
      this.config[config.codec] = {
        format: 'annexb',
      };
    }
    this.decoder.configure({
      ...config,
      codec: this.getCodec(config)
    });
  }
  getCodec(config: VideoDecoderConfig) {
    switch (config.codec) {
      case "hevc":
        return 'hvc1.1.6.L0.12.34.56.78.9A.BC';
      case "av1":
        return 'av01.0.05M.08';
      case "avc":
        return 'avc1.420028';
      default:
        return config.codec;
    }
  }
  @Includes("configured")
  decode(packet: EncodedVideoChunkInit): void {
    if (this.decoder.state === "configured")
      this.decoder.decode(new EncodedVideoChunk(packet));
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
    if (this.decoder.state !== "closed")
      this.decoder.close();
  }
}
