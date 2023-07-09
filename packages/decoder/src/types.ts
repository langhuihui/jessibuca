import EventEmitter from "eventemitter3";

export type DecoderState = 'uninitialized' | 'initialized' | 'configured' | 'closed';

//视频解码器类型
export type VideoDecoderType = 'soft' | 'soft-simd' | 'hard' | 'auto';


export interface VideoCodecInfo {

  width: number,
  height: number;
};


export interface VideoPacket {

  data: BufferSource,
  keyFrame: boolean,
  pts: number;
};

export interface JVideoFrame {

  pixelType: VideoPixelFormat,
  data: Uint8Array,
  width: number,
  height: number,
  pts: number;
};

export interface ErrorInfo {

  errMsg: string;
}

export const enum VideoDecoderEvent {
  VideoCodecInfo = "videoCodecInfo",
  VideoFrame = "videoFrame",
  Error = "error"
};

export interface VideoDecoderInterface extends EventEmitter {
  initialize(): Promise<void>;
  configure(config: VideoDecoderConfig): void;
  decode(packet: EncodedVideoChunkInit): void;
  flush(): void;
  reset(): void;
  close(): void;
};

//声音压缩格式
export type AudioCodec = 'pcma' | 'pcmu' | 'aac' | 'opus' | 'unknow' | string;



export interface AudioCodecInfo {

  sampleRate: number,
  channels: number,
  depth: number,
  aac?: {
    profile: number;
  };

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

export interface AudioDecoderInterface extends EventEmitter {
  initialize(): Promise<void>;
  configure(config: AudioDecoderConfig): void;
  decode(packet: EncodedAudioChunkInit): void;
  flush(): void;
  reset(): void;
  close(): void;
};
