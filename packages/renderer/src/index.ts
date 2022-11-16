import { EventEmitter } from "eventemitter3";
import { CanvasRenderer } from "./canvas";
import { MSEVideoRenderer, WebCodecsVideoRenderer } from "./video";
export interface RendererCapabilities {
  webCodecs?: boolean; //chrome >94
}
export class RendererError extends Error {}
export class Renderer extends EventEmitter {
  static capabilities: RendererCapabilities = {
    webCodecs: typeof MediaStreamTrackGenerator != "undefined",
  };
  static EdataType = new RendererError("data type note support");
  renderer: WebCodecsVideoRenderer | MSEVideoRenderer | CanvasRenderer;
  constructor(public display: HTMLVideoElement | HTMLCanvasElement) {
    super();
    if (display instanceof HTMLVideoElement) {
      if (Renderer.capabilities.webCodecs) {
        this.renderer = new WebCodecsVideoRenderer(display);
      } else {
        this.renderer = new MSEVideoRenderer(display);
      }
    } else {
      this.renderer = new CanvasRenderer(display);
    }
  }
  writeAudio(data: AudioData | Uint8Array, init?: AudioDataInit) {}
  writeVideo(frame: VideoFrame | Uint8Array, init?: VideoFrameBufferInit) {
    if (frame instanceof Uint8Array) {
      if (this.renderer instanceof WebCodecsVideoRenderer) {
        this.renderer.writeVideo(new VideoFrame(frame, init!));
      } else {
        this.renderer.writeVideo(frame, init!);
      }
    } else {
      if (this.renderer instanceof WebCodecsVideoRenderer) {
        this.renderer.writeVideo(frame);
      } else {
        const buffer = new Uint8Array(frame.allocationSize());
        frame.copyTo(buffer);
        this.renderer.writeVideo(buffer, init!);
        frame.close();
      }
    }
  }
  close() {
    this.renderer.close();
  }
}
