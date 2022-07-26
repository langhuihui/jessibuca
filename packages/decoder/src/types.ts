

//视频解码器类型
export const enum VideoDecoderType {

    SoftwareDecoder             = 1, //软解码器
    SoftwareDecoderWithMT       = 2, //支持多线程软解码器
    SoftwareDecoderWithSIMD     = 3, //支持SIMD软解码器
    SoftwareDecoderWithSIMDMT   = 4, //支持SIMD和多线程的软解码器
    HardwareDecoder             = 20, //硬解码器
    AutoDecoder                 = 100,//自动选择解码器
}

export const enum AudioDecoderType {

    SoftwareDecoder     = 1, //软解码器
    AutoDecoder         = 100,//自动选择解码器
}


//视频压缩格式
export const enum VideoType {
    AVC  = 1, //H264
    HEVC = 2, //H265
}

//像素格式
export const enum PixelType {
    YUV420P  = 1, //I420
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