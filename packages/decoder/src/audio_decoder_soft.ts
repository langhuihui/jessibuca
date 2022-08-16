import EventEmitter from 'eventemitter3';
import { DecoderState, AudioDecoderConfig, AudioPacket, AudioDecoderInterface, AudioCodecInfo, AudioDecoderEvent, AudioFrame, ErrorInfo } from './types';
import CreateModule, { WASMModule } from '../wasm/out/decoderaudio';
import SpliteBuffer from './splitebuffer';

export class AudioSoftDecoder extends EventEmitter implements AudioDecoderInterface {

    decoderState: DecoderState;
    decoder: any;
    config?: AudioDecoderConfig;
    module?: WASMModule;

    sampleRate: number;
    channels: number;
    useSpliteBuffer: boolean;
    spliteBuffer?: SpliteBuffer;

    constructor() {

        super();
        this.decoderState = 'uninitialized';
        this.sampleRate = 0;
        this.channels = 0;
        this.useSpliteBuffer = false;

    };

    initialize(): Promise<void>{

        return new Promise(resolve => {

            const opts: any = {};
            opts.print = ((text: string) => console.log(text));
            opts.printErr = ((text: string) => console.log(`[JS] ERROR: ${text}`));
            opts.onAbort = (() => console.log("[JS] FATAL: WASM ABORTED"));
            opts.postRun = ((m: WASMModule) => {

                this.module = m;
         
                this.decoder = new this.module.AudioDecoder(this);
                this.decoderState = 'initialized';

                resolve();

            })

            console.log(`audio soft decoder initialize call`);
    
            CreateModule(opts);

        })

    }

    state(): DecoderState {

        return this.decoderState;

    }

    configure(config: AudioDecoderConfig): void {

        if (this.decoderState !== 'initialized') {

            console.warn(`the decoder not initialized`);
            return;
        }

        this.config = config;

        this.decoder.setCodec(this.config.audioType, this.config.extraData);
        this.decoderState = 'configured';

    }
    decode(packet: AudioPacket): void {

        if (this.decoderState !== 'configured') {

            console.warn(`the decoder not configured`);
            return;
        }

        this.decoder.decode(packet.data,  packet.pts);

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
    audioInfo(sampleRate: number, channels: number): void {

        this.sampleRate = sampleRate;
        this.channels = channels;

        let audioCodeInfo : AudioCodecInfo = {
            sampleRate,
            channels,
            depth:16
        };

        this.emit(AudioDecoderEvent.AudioCodecInfo, audioCodeInfo);
        
    }


    pcmData(pcmDataArray: number, samples: number, pts: number): void {

         if (!this.module) {

            return;
         }

         let pcmDatas: Float32Array[] = [];

         for (let i = 0; i < this.channels; i++) {
             let fp = this.module.HEAPU32[(pcmDataArray >> 2) + i] >> 2;
             pcmDatas.push(Float32Array.of(...this.module.HEAPF32.subarray(fp, fp + samples)));
         }


         if (!this.useSpliteBuffer) {

             if(samples === this.config?.outSampleNum) {

                let aFrame: AudioFrame = {
                    datas: pcmDatas,
                    sampleNum: samples,
                    channles: this.channels,
                    pts: pts,
                }

                this.emit(AudioDecoderEvent.AudioFrame, aFrame);

                 return;
             }

             this.spliteBuffer = new SpliteBuffer(this.sampleRate, this.channels, this.config?.outSampleNum ?? 1024);
             this.useSpliteBuffer = true;
         } 

         this.spliteBuffer?.addBuffer(pcmDatas, pts);

         this.spliteBuffer?.splite((buffers, ts) => {

             let aFrame: AudioFrame = {
                datas: buffers,
                sampleNum: this.config?.outSampleNum ?? 1024,
                channles: this.channels,
                pts: ts,
            }

            this.emit(AudioDecoderEvent.AudioFrame, aFrame);

         });

    }

    errorInfo(errormsg: string): void {

        let err : ErrorInfo = {
            errMsg: errormsg
        };

        this.emit(AudioDecoderEvent.Error, err);
    }


};
