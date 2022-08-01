import EventEmitter from 'eventemitter3';
import { DecoderState, VideoDecoderConfig, VideoPacket, VideoDecoderInterface, VideoCodecInfo, VideoDecoderEvent } from './types';
import CreateModule, { WASMModule } from '../wasm/out/decodervideo';


export class VideoSoftDecoder extends EventEmitter implements VideoDecoderInterface {

    decoderState: DecoderState;
    decoder: any;
    config: undefined | VideoDecoderConfig;
    module: undefined |WASMModule;

    width: number;
    height: number;

    constructor(useSIMD: boolean) {

        super();
        this.decoderState = 'uninitialized';
        this.width = 0;
        this.height = 0;

    };

    initialize(): Promise<void>{

        return new Promise(resolve => {

            const opts: any = {};
            opts.print = ((text: string) => console.log(text));
            opts.printErr = ((text: string) => console.log(`[JS] ERROR: ${text}`));
            opts.onAbort = (() => console.log("[JS] FATAL: WASM ABORTED"));
            opts.postRUun = (() => {

                if (this.module) {

                    this.decoder = new this.module.VideoDecoder(this);
                }

                this.decoderState = 'initialized';

                resolve();

            })
    
            this.module = CreateModule(opts);

        })

    }

    state(): DecoderState {

        return this.decoderState;

    }

    configure(config: VideoDecoderConfig): void {

        if (this.decoderState !== 'initialized') {

            console.warn(`the decoder not initialized`);
            return;
        }

        this.config = config;
        this.decoder.setCodec(this.config.videoType, this.config.extraData);
        this.decoderState = 'configured';

    }
    decode(packet: VideoPacket): void {

        if (this.decoderState !== 'configured') {

            console.warn(`the decoder not configured`);
            return;
        }

        this.decoder.decode(packet.data, packet.keyFrame, packet.pts);

    }

    flush(): void {

    }

    reset(): void {

        if (this.decoderState === 'uninitialized' || this.decoderState === 'closed') {

            return;
        }

        this.config = undefined;
        if (this.decoder) {
            this.decoder.clear();
        }
        this.decoderState = 'initialized';

    }

    close(): void {

        this.removeAllListeners();

        if (this.decoder) {
            this.decoder.clear();
            this.decoder.delete();
        }

        this.decoderState = 'closed';
    }

    // wasm callback function
    videoInfo(vtype: number, width: number, height: number): void {

        this.width = width;
        this.height = height;

        let videoCodeInfo : VideoCodecInfo = {
            videoType: vtype === 1 ? 'avc' : 'hevc',
            width: width,
            height: height
        };

        this.emit(VideoDecoderEvent.VideoCodecInfo, videoCodeInfo);
        
    }


    yuvData(yuv: string, pts: number): void {

         let size = this.width*this.height*3/2;


         if (this.module) {

            let out = this.module.HEAPU8.subarray(yuv, yuv + size);
         }

        

        // let data = Uint8Array.from(out);

        // this._yuvframerate++;
        // this._yuvbitrate += data.length;

        
      

    }

};
