import { VideoDecoderInterface, VideoCodecInfo, VideoDecoderEvent, JVideoFrame, ErrorInfo } from './types';
import { FSM, ChangeState, Includes } from 'afsm';

function WorkerScripts() {
  var decoder;
  self.onmessage = (evt) => {
    if (evt.data.type === 'init') {
      const { canvas, wasmScript, wasmBinary } = evt.data;
      const gl = canvas?.getContext('2d');
      let width = 0;
      let height = 0;
      const module = {
        wasmBinary,
        postRun: () => {
          decoder = new module.VideoDecoder({
            videoInfo(w: number, h: number): void {
              width = w;
              height = h;
              console.log("video info", w, h);
            },
            yuvData(yuvArray: number, pts: number): void {
              const size = width * height;
              const halfSize = size >> 2;

              let yPtr = module.HEAPU32![(yuvArray >> 2)];
              let uPtr = module.HEAPU32![(yuvArray >> 2) + 1];
              let vPtr = module.HEAPU32![(yuvArray >> 2) + 2];

              let yBuf = module.HEAPU8!.subarray(yPtr, yPtr + size);
              let uBuf = module.HEAPU8!.subarray(uPtr, uPtr + halfSize);
              let vBuf = module.HEAPU8!.subarray(vPtr, vPtr + halfSize);
              const data = new Uint8Array(size + halfSize + halfSize);
              data.set(yBuf);
              data.set(uBuf, size);
              data.set(vBuf, size + halfSize);
              const videoFrame = new VideoFrame(data, {
                codedWidth: width,
                codedHeight: height,
                format: 'I420',
                timestamp: pts
              });
              if (canvas) {
                gl?.drawImage(videoFrame, 0, 0, canvas.width, canvas.height);
                gl?.commit();
              } else {
                self.postMessage({ type: 'yuvData', videoFrame }, [videoFrame]);
              }
            }
          });
          self.postMessage({ type: 'ready' });
        }
      };
      Function("var _scriptDir = typeof document !== 'undefined' && document.currentScript ? document.currentScript.src : undefined;return " + wasmScript)()(module);
    } else if (evt.data.type === 'decode') {
      const { packet } = evt.data;
      decoder?.decode(packet.data, packet.type == 'key', packet.timestamp);
    } else if (evt.data.type === 'setCodec') {
      const { codec, format, description } = evt.data;
      decoder?.setCodec(codec, format, description ?? '');
    }
  };
}

export class VideoDecoderSoftBase extends FSM implements VideoDecoderInterface {
  worker: Worker;
  decoder: any;
  config?: VideoDecoderConfig;
  module: Partial<EmscriptenModule & { VideoDecoder: any; }> = {};
  width = 0;
  height = 0;

  constructor(public createModule: any, private wasmBinary?: Promise<ArrayBuffer>, private workerMode = false, private canvas?: HTMLCanvasElement, private yuvMode = false) {
    super();
  };
  @ChangeState([FSM.INIT, "closed"], "initialized")
  async initialize(opt?: EmscriptenModule): Promise<void> {
    if (this.workerMode) {
      const script = /\{(.+)\}/s.exec(WorkerScripts.toString())![1];
      this.worker = new Worker(URL.createObjectURL(new Blob([script], { type: 'text/javascript' })));
      const offsetCanvas = this.canvas?.transferControlToOffscreen();
      const wasmBinary = await this.wasmBinary;
      console.warn("worker mode", wasmBinary);
      this.worker.postMessage({ type: 'init', canvas: offsetCanvas, wasmScript: this.createModule.toString(), wasmBinary }, offsetCanvas ? [offsetCanvas, wasmBinary] : [wasmBinary]);
      return new Promise(resolve => {
        this.worker.onmessage = (evt) => {
          if (evt.data.type === 'ready') {
            delete this.wasmBinary;
            resolve();
            console.warn(`worker mode initialize success`);
          } else if (evt.data.type === 'yuvData') {
            const { videoFrame } = evt.data;
            this.emit(VideoDecoderEvent.VideoFrame, videoFrame);
          }
        };
      });
    }
    const opts: any = this.module;
    if (this.wasmBinary) {
      opts.wasmBinary = await this.wasmBinary;
    }
    opts.print = ((text: string) => console.log(text));
    opts.printErr = ((text: string) => console.log(`[JS] ERROR: ${text}`));
    opts.onAbort = (() => console.log("[JS] FATAL: WASM ABORTED"));
    return new Promise(resolve => {
      opts.postRun = ((m: any) => {
        this.decoder = new this.module.VideoDecoder(this);
        console.log(`video soft decoder initialize success`);
        resolve();
      });
      if (opt) Object.assign(opts, opt);
      this.createModule(opts);
    });
  }
  @ChangeState("initialized", "configured", { sync: true })
  configure(config: VideoDecoderConfig): void {
    this.config = config;
    const codec = this.config.codec.startsWith('avc') ? 'avc' : 'hevc';
    const format = this.config.description ? (codec == 'avc' ? 'avcc' : 'hvcc') : 'annexb';
    this.decoder?.setCodec(codec, format, this.config.description ?? '');
    this.worker?.postMessage({ type: 'setCodec', codec, format, description: this.config.description });
  }
  decode(packet: EncodedVideoChunkInit): void {
    this.decoder?.decode(packet.data, packet.type == 'key', packet.timestamp);
    if (this.state === "configured") this.worker?.postMessage({ type: 'decode', packet });
  }

  flush(): void {

  }
  @ChangeState([], FSM.INIT, { sync: true })
  reset(): void {
    this.config = undefined;
    if (this.decoder) {
      this.decoder.clear();
    }
  }
  @ChangeState([], "closed", { sync: true })
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
    if (this.yuvMode) {
      this.emit(VideoDecoderEvent.VideoFrame, [yBuf, uBuf, vBuf]);
      return;
    }
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
