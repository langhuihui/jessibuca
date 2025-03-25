declare module "types" {
    import EventEmitter from "eventemitter3";
    export type DecoderState = 'uninitialized' | 'initialized' | 'configured' | 'closed';
    export type VideoDecoderType = 'soft' | 'soft-simd' | 'hard' | 'auto';
    export interface VideoCodecInfo {
        width: number;
        height: number;
    }
    export interface VideoPacket {
        data: BufferSource;
        keyFrame: boolean;
        pts: number;
    }
    export interface JVideoFrame {
        pixelType: VideoPixelFormat;
        data: Uint8Array;
        width: number;
        height: number;
        pts: number;
    }
    export interface ErrorInfo {
        errMsg: string;
    }
    export const enum VideoDecoderEvent {
        VideoCodecInfo = "videoCodecInfo",
        VideoFrame = "videoFrame",
        Error = "error"
    }
    export interface VideoDecoderInterface extends EventEmitter {
        initialize(opt?: EmscriptenModule): Promise<void>;
        configure(config: VideoDecoderConfig): void;
        decode(packet: EncodedVideoChunkInit): void;
        flush(): void;
        reset(): void;
        close(): void;
    }
    export type AudioCodec = 'pcma' | 'pcmu' | 'aac' | 'opus' | 'unknow' | string;
    export interface AudioCodecInfo {
        sampleRate: number;
        channels: number;
        depth: number;
        aac?: {
            profile: number;
        };
    }
    export interface AudioFrame {
        datas: BufferSource[];
        sampleNum: number;
        channles: number;
        pts: number;
    }
    export const enum AudioDecoderEvent {
        AudioCodecInfo = "audioCodecInfo",
        AudioFrame = "audioFrame",
        Error = "error"
    }
    export interface AudioDecoderInterface extends EventEmitter {
        initialize(): Promise<void>;
        configure(config: AudioDecoderConfig): void;
        decode(packet: EncodedAudioChunkInit): void;
        flush(): void;
        reset(): void;
        close(): void;
    }
}
declare module "video_decoder_soft_base" {
    import { VideoDecoderInterface, VideoCodecInfo, VideoDecoderEvent, ErrorInfo } from "types";
    import { FSM } from 'afsm';
    export class VideoDecoderSoftBase extends FSM<{
        [VideoDecoderEvent.VideoCodecInfo]: [VideoCodecInfo];
        [VideoDecoderEvent.VideoFrame]: [VideoFrame | {
            y: Uint8Array;
            u: Uint8Array;
            v: Uint8Array;
            timestamp: number;
        }];
        [VideoDecoderEvent.Error]: [ErrorInfo];
    }> implements VideoDecoderInterface {
        createModule: any;
        private wasmBinary?;
        private workerMode;
        private canvas?;
        private yuvMode;
        worker: Worker;
        decoder: any;
        config?: VideoDecoderConfig;
        module: Partial<EmscriptenModule & {
            VideoDecoder: any;
        }>;
        width: number;
        height: number;
        constructor(createModule: any, wasmBinary?: Promise<ArrayBuffer>, workerMode?: boolean, canvas?: HTMLCanvasElement, yuvMode?: boolean);
        initialize(opt?: EmscriptenModule): Promise<void>;
        configure(config: VideoDecoderConfig): void;
        decode(packet: EncodedVideoChunkInit): void;
        flush(): void;
        reset(): void;
        close(): void;
        videoInfo(width: number, height: number): void;
        yuvData(yuvArray: number, pts: number): void;
        errorInfo(errormsg: string): void;
    }
}
declare module "video_decoder_soft" {
    import { VideoDecoderSoftBase } from "video_decoder_soft_base";
    export class VideoDecoderSoft extends VideoDecoderSoftBase {
        constructor(opt?: {
            workerMode?: boolean;
            yuvMode?: boolean;
            canvas?: HTMLCanvasElement;
            wasmPath?: string;
        });
    }
}
declare module "video_decoder_soft_simd" {
    import { VideoDecoderSoftBase } from "video_decoder_soft_base";
    export class VideoDecoderSoftSIMD extends VideoDecoderSoftBase {
        constructor(opt?: {
            workerMode?: boolean;
            canvas?: HTMLCanvasElement;
            wasmPath?: string;
            yuvMode?: boolean;
        });
    }
}
declare module "video_decoder_hard" {
    import { VideoDecoderInterface } from "types";
    import { FSM } from "afsm";
    export class VideoDecoderHard extends FSM implements VideoDecoderInterface {
        decoder: VideoDecoder;
        config?: VideoDecoderConfig;
        initialize(): Promise<void>;
        configure(config: VideoDecoderConfig): void;
        getCodec(config: VideoDecoderConfig): string;
        decode(packet: EncodedVideoChunkInit): void;
        flush(): void;
        reset(): void;
        close(): void;
    }
}
declare module "fmp4" {
    interface MP4Types {
        [key: string]: number[];
    }
    interface MP4Constants {
        FTYP: Uint8Array;
        STSD_PREFIX: Uint8Array;
        STTS: Uint8Array;
        STSC: Uint8Array;
        STCO: Uint8Array;
        STSZ: Uint8Array;
        HDLR_VIDEO: Uint8Array;
        HDLR_AUDIO: Uint8Array;
        DREF: Uint8Array;
        SMHD: Uint8Array;
        VMHD: Uint8Array;
    }
    export interface MP4Meta {
        id: number;
        type: 'audio' | 'video';
        timescale: number;
        duration: number;
        codecWidth: number;
        codecHeight: number;
        presentWidth: number;
        presentHeight: number;
        channelCount: number;
        audioSampleRate: number;
        config?: number[];
        avcc: Uint8Array;
        videoType?: string;
    }
    interface MP4Track {
        id: number;
        sequenceNumber: number;
        duration: number;
        size: number;
        flags: {
            isLeading: number;
            dependsOn: number;
            isDependedOn: number;
            hasRedundancy: number;
            isNonSync: number;
        };
        cts: number;
    }
    class MP4 {
        static types: MP4Types;
        static readonly constants: MP4Constants;
        static box(type: number[], ...datas: Uint8Array[]): Uint8Array;
        static generateInitSegment(meta: MP4Meta): Uint8Array;
        static moov(meta: MP4Meta): Uint8Array;
        static mvhd(timescale: number, duration: number): Uint8Array;
        static trak(meta: MP4Meta): Uint8Array;
        static tkhd(meta: MP4Meta): Uint8Array;
        static mdia(meta: MP4Meta): Uint8Array;
        static mdhd(meta: MP4Meta): Uint8Array;
        static hdlr(meta: MP4Meta): Uint8Array;
        static minf(meta: MP4Meta): Uint8Array;
        static dinf(): Uint8Array;
        static stbl(meta: MP4Meta): Uint8Array;
        static stsd(meta: MP4Meta): Uint8Array;
        static mp4a(meta: MP4Meta): Uint8Array;
        static esds(meta: MP4Meta): Uint8Array;
        static avc1(meta: MP4Meta): Uint8Array;
        static hvc1(meta: MP4Meta): Uint8Array;
        static mvex(meta: MP4Meta): Uint8Array;
        static trex(meta: MP4Meta): Uint8Array;
        static moof(track: MP4Track, baseMediaDecodeTime: number): Uint8Array;
        static mfhd(sequenceNumber: number): Uint8Array;
        static traf(track: MP4Track, baseMediaDecodeTime: number): Uint8Array;
        static sdtp(track: MP4Track): Uint8Array;
        static trun(track: MP4Track, offset: number): Uint8Array;
        static mdat(data: Uint8Array): Uint8Array;
    }
    export default MP4;
}
declare module "exp-golomb" {
    class ExpGolomb {
        private readonly TAG;
        private _buffer;
        private _buffer_index;
        private _total_bytes;
        private _total_bits;
        private _current_word;
        private _current_word_bits_left;
        constructor(uint8array: Uint8Array);
        destroy(): void;
        private _fillCurrentWord;
        readBits(bits: number): number;
        readBool(): boolean;
        readByte(): number;
        private _skipLeadingZero;
        readUEG(): number;
        readSEG(): number;
    }
    export default ExpGolomb;
}
declare module "h264-sps-parser" {
    import ExpGolomb from "exp-golomb";
    class SPSParser {
        static _ebsp2rbsp(uint8array: Uint8Array): Uint8Array;
        static parseSPS(uint8array: Uint8Array): {
            codec_mimetype: string;
            profile_idc: number;
            level_idc: number;
            profile_string: string;
            level_string: string;
            chroma_format_idc: number;
            bit_depth: number;
            bit_depth_luma: number;
            bit_depth_chroma: number;
            ref_frames: number;
            chroma_format: number;
            chroma_format_string: string;
            frame_rate: {
                fixed: boolean;
                fps: number;
                fps_den: number;
                fps_num: number;
            };
            sar_ratio: {
                width: number;
                height: number;
            };
            codec_size: {
                width: number;
                height: number;
            };
            present_size: {
                width: number;
                height: number;
            };
        };
        static _skipScalingList(gb: ExpGolomb, count: number): void;
        static getProfileString(profile_idc: number): string;
        static getLevelString(level_idc: number): string;
        static getChromaFormatString(chroma: number): string;
    }
    export default SPSParser;
}
declare module "h264" {
    import { MP4Meta } from "fmp4";
    export function parseAVCDecoderConfigurationRecord(arrayBuffer: Uint8Array): MP4Meta;
    export function avcEncoderConfigurationRecord({ sps, pps }: {
        sps: Uint8Array;
        pps: Uint8Array;
    }): Uint8Array<ArrayBuffer>;
}
declare module "video_decoder_mse" {
    import { VideoDecoderInterface } from "types";
    import { FSM } from "afsm";
    export class VideoDecoderMSE extends FSM implements VideoDecoderInterface {
        sourceBuffer?: SourceBuffer;
        sequenceNumber: any;
        sourceBufferCache: Uint8Array[];
        decode(packet: EncodedVideoChunkInit): void;
        flush(): void;
        reset(): void;
        close(): void;
        mse: MediaSource;
        src?: string;
        element?: HTMLVideoElement;
        config?: VideoDecoderConfig;
        initialize(videoElement: HTMLVideoElement): any;
        configure(config: VideoDecoderConfig): void;
    }
}
declare module "audio_decoder_soft" {
    import { AudioDecoderInterface } from "types";
    import { FSM } from "afsm";
    export class AudioDecoderSoft extends FSM implements AudioDecoderInterface {
        decoder: any;
        config?: AudioDecoderConfig;
        module?: any;
        sampleRate: number;
        channels: number;
        initialize(): Promise<void>;
        configure(config: AudioDecoderConfig): void;
        decode(packet: EncodedAudioChunkInit): void;
        flush(): void;
        reset(): void;
        close(): void;
        audioInfo(sampleRate: number, channels: number): void;
        pcmData(pcmDataArray: number, samples: number, pts: number): void;
        errorInfo(errormsg: string): void;
    }
}
declare module "audio_decoder_hard" {
    import { AudioDecoderInterface } from "types";
    import { FSM } from "afsm";
    export class AudioDecoderHard extends FSM implements AudioDecoderInterface {
        decoder: AudioDecoder;
        config?: AudioDecoderConfig;
        initialize(): Promise<void>;
        configure(config: AudioDecoderConfig): void;
        decode(packet: EncodedAudioChunkInit): void;
        flush(): void;
        reset(): void;
        close(): void;
    }
}
declare module "index" {
    export * from "video_decoder_soft";
    export * from "video_decoder_soft_simd";
    export * from "video_decoder_hard";
    export * from "video_decoder_mse";
    export * from "audio_decoder_soft";
    export * from "audio_decoder_hard";
}
