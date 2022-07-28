//视频解码器类型
export const enum VideoDecoderType {

    SoftwareDecoder             = 1, //软解码器
    SoftwareDecoderWithSIMD     = 2, //支持SIMD软解码器
    HardwareDecoder             = 20, //硬解码器
    AutoDecoder                 = 100,//自动选择解码器
}


//视频参数相关定义

//视频压缩格式
export const enum VideoType {
    AVC  = 1, //H264
    HEVC = 2, //H265
}

//视频数据格式
export const enum VideoDataType {
    AVCC   = 1, //Nal 前4个字节是Nal长度
    AnnexB = 2, //Nal 前4个字节是0 0 0 1
}

//像素格式
export const enum PixelType {
    YUV420P  = 1, //I420
}


export type VideoCodecInputInfo = {

    videoType: VideoType,
    extraData: undefined | Uint8Array,
    outPixelType?: PixelType

}

export type VideoCodecOutputInfo = {

    videoType: VideoType,
    width: number,
    height: number
}


export type VideoPacket = {

    videoDataType: VideoDataType,
    data: Uint8Array,
    keyFrame: boolean,
    pts: number
}

export type VideoFrame = {

    pixelType: PixelType,
    datas: Uint8Array[],
    width: number,
    height: number,
    pts: number
}




export const enum VideoDecoderEvent {
    VideoCodecInfo = "videoCodecOutInfo",
    VideoFrame = "videoFrame",
    Error = "error"
  }

}

//audio 参数


export const enum AudioDecoderType {

    SoftwareDecoder     = 1, //软解码器
    AutoDecoder         = 100,//自动选择解码器
}




//声音压缩格式
export const enum AuidoType {
    PCMA  = 1, 
    PCMU  = 2,
    AAC   = 3,
}

//PCM 采样格式
export const enum SampleType {
    FLTP  = 1, //4 bytes float, [-1, 1]
    PCM16 = 2, //2 bytes short, [-32768, 32767]
}


export type AudioCodecInputInfo = {

    audioType: AuidoType,
    extraData: undefined | Uint8Array,
    outSampleType?: SampleType,
    outSampleNum?:number //按指定采样点个数输出，内部会做队列缓存

}

export type AudioCodecOutputInfo = {

    audioType: AuidoType,
    sampleRate: number,
    channles: number,
    depth: number,
    profile?: number  //如果是aac 才有profile 

}


export type AudioPacket = {

    data: Uint8Array,
    pts: number

}

export type AudioFrame = {

    datas: Uint16Array[] | Float32Array[],
    sampleRate: number,
    channles: number,
    pts: number,

}




export const enum AudioDecoderEvent {
    AudioCodecInfo = "videoCodecOutInfo",
    VideoFrame = "videoFrame",
    Error = "error"
}