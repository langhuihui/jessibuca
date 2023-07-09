import { VideoDecoderInterface, VideoCodecInfo, VideoDecoderEvent, JVideoFrame, ErrorInfo } from './types';
import { FSM, ChangeState, Includes } from 'afsm';


export class VideoDecoderSoftBase extends FSM implements VideoDecoderInterface {

  decoder: any;
  config?: VideoDecoderConfig;
  module: Partial<EmscriptenModule & { VideoDecoder: any; }> = {};
  createModule: any;

  width: number;
  height: number;

  constructor(createModule: any) {

    super();
    this.width = 0;
    this.height = 0;
    this.createModule = createModule;

  };
  @ChangeState([FSM.INIT, "closed"], "initialized")
  initialize(): Promise<void> {
    return new Promise(resolve => {
      const opts: any = this.module;
      opts.print = ((text: string) => console.log(text));
      opts.printErr = ((text: string) => console.log(`[JS] ERROR: ${text}`));
      opts.onAbort = (() => console.log("[JS] FATAL: WASM ABORTED"));
      opts.postRun = ((m: any) => {
        this.decoder = new this.module.VideoDecoder(this);
        console.log(`video soft decoder initialize success`);
        resolve();
      });
      this.createModule(opts);
    });

  }
  @ChangeState("initialized", "configured")
  configure(config: VideoDecoderConfig): void {
    this.config = config;
    this.decoder.setCodec(this.config.codec, this.config.description ? 'avcc' : 'annexb', this.config.description ?? '');
  }
  @Includes("configured")
  decode(packet: EncodedVideoChunkInit): void {
    this.decoder.decode(packet.data, packet.type == 'key', packet.timestamp);
  }

  flush(): void {

  }
  @ChangeState([], FSM.INIT)
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
  videoInfo(width: number, height: number): void {

    this.width = width;
    this.height = height;

    let videoCodeInfo: VideoCodecInfo = {
      width: width,
      height: height
    };

    this.emit(VideoDecoderEvent.VideoCodecInfo, videoCodeInfo);

  }


  yuvData(yuvArray: number, pts: number): void {

    if (!this.module) {

      return;
    }

    const size = this.width * this.height;
    const halfSize = size >> 2;

    let yPtr = this.module.HEAPU32![(yuvArray >> 2)];
    let uPtr = this.module.HEAPU32![(yuvArray >> 2) + 1];
    let vPtr = this.module.HEAPU32![(yuvArray >> 2) + 2];

    let yBuf = this.module.HEAPU8!.subarray(yPtr, yPtr + size);
    let uBuf = this.module.HEAPU8!.subarray(uPtr, uPtr + halfSize);
    let vBuf = this.module.HEAPU8!.subarray(vPtr, vPtr + halfSize);
    const data = new Uint8Array(size + halfSize + halfSize);
    data.set(yBuf);
    data.set(uBuf, size);
    data.set(vBuf, size + halfSize);
    //  let datas = [Uint8Array.from(yBuf), Uint8Array.from(uBuf), Uint8Array.from(vBuf)];

    // let vFrame: JVideoFrame = {
    //   pixelType: 'I420',
    //   data,
    //   width: this.width,
    //   height: this.height,
    //   pts: pts
    // };

    this.emit(VideoDecoderEvent.VideoFrame, new VideoFrame(data, {
      codedWidth: this.width,
      codedHeight: this.height,
      format: 'I420',
      timestamp: pts
    }));

  }

  errorInfo(errormsg: string): void {

    let err: ErrorInfo = {
      errMsg: errormsg
    };

    this.emit(VideoDecoderEvent.Error, err);
  }

};
