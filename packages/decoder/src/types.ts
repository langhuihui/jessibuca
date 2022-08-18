
export type DecoderState = 'uninitialized' | 'initialized' | 'configured' | 'closed';

//视频解码器类型
export type VideoDecoderType = 'soft' | 'soft-simd' | 'hard' | 'auto';

//视频压缩格式
export type  VideoType = 'avc' | 'hevc' | 'unknow';

//像素格式
export type PixelType = 'I420';

//视频解码器配置
export type VideoDecoderConfig = {

    videoType: VideoType,
    extraData?: BufferSource,
    avc?:{
        format: "avcc" | "annexb";
    },
    hevc?:{
        format: "hvcc" | "annexb";
    }
    outPixelType?: PixelType,

};

export interface VideoCodecInfo {

    width: number,
    height: number
};


export interface VideoPacket {

    data: BufferSource,
    keyFrame: boolean,
    pts: number
};

export interface VideoFrame {

    pixelType: PixelType,
    datas: BufferSource[],
    width: number,
    height: number,
    pts: number
};

export interface ErrorInfo {

    errMsg:string
}

export const enum VideoDecoderEvent {
    VideoCodecInfo = "videoCodecInfo",
    VideoFrame = "videoFrame",
    Error = "error"
};

export interface VideoDecoderInterface  {

    initialize():Promise<void>;
    state(): DecoderState;
    configure(config: VideoDecoderConfig): void;
    decode(packet: VideoPacket): void;
    flush(): void;
    reset(): void;
    close(): void;

};

//audio 参数
export type AudioDecoderType = 'soft' | 'hard' | 'auto';

//声音压缩格式
export type AuidoType  =  'pcma' | 'pcmu' | 'aac' | 'opus' | 'unknow';

export type SampleType = 'f32-planar';

export interface AudioDecoderConfig  {

    audioType: AuidoType,
    extraData?: BufferSource,
    outSampleType?: SampleType
}

export interface AudioCodecInfo {

    sampleRate: number,
    channels: number,
    depth: number,
    aac?: {
        profile: number
    }

}

export interface AudioPacket {

    data: BufferSource,
    pts: number

}

export interface AudioFrame {

    datas: BufferSource[],
    sampleNum: number,
    channles: number,
    pts: number,

}

export const enum AudioDecoderEvent {
    AudioCodecInfo = "audioCodecInfo",
    AudioFrame = "audioFrame",
    Error = "error"
}

export interface AudioDecoderInterface {

    state(): DecoderState;
    initialize():Promise<void>;
    configure(config: AudioDecoderConfig): void;
    decode(packet: AudioPacket): void;
    flush(): void;
    reset(): void;
    close(): void;

};
