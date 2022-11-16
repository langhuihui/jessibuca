import {
  VideoDecoderConfig,
  VideoDecoderInterface,
  VideoDecoderEvent,
} from "./types";
import { ChangeState, FSM, Includes } from "afsm";
export class VideoDecoderHard extends FSM implements VideoDecoderInterface {
  decoder!: VideoDecoder;
  @ChangeState(FSM.INIT, "initialized")
  async initialize() {
    this.decoder = new VideoDecoder({
      output: (frame) => {
        this.emit(VideoDecoderEvent.VideoFrame, frame);
      },
      error: (err) => this.emit(VideoDecoderEvent.Error, err),
    });
  }
  @ChangeState("initialized", "configured")
  configure(config: VideoDecoderConfig): void {
    this.decoder.configure({
      codec: config.codec,
      description: config.extraData,
    });
  }
  @Includes("configured")
  decode(packet: EncodedVideoChunkInit): void {
    this.decoder.decode(new EncodedVideoChunk(packet));
  }
  flush(): void {
    this.decoder.flush();
  }
  reset(): void {
    this.decoder.reset();
  }
  @ChangeState([], "closed")
  close(): void {
    this.decoder.close();
  }
}
