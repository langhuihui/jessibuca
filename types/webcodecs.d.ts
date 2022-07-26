interface AudioDecoder {
  new(init: AudioDecoderInit);
  readonly state: CodecState;
  readonly decodeQueueSize: number;
  configure(config: AudioDecoderConfig): void;
  decode(chunk: EncodedAudioChunk): void;
  flush(): Promise<void>;
  reset(): void;
  close(): void;
  isConfigSupported(config: AudioDecoderConfig): Promise<AudioDecoderSupport>;
}
interface AudioDecoderInit {
  output: AudioDataOutputCallback;
  error: WebCodecsErrorCallback;
}
type AudioDataOutputCallback = (output: AudioData) => void;
interface VideoDecoder {
  new(init: VideoDecoderInit);
  readonly state: CodecState;
  readonly decodeQueueSize: number;
  configure(config: VideoDecoderConfig): void;
  decode(chunk: EncodedVideoChunk): void;
  flush(): Promise<void>;
  reset(): void;
  close(): void;
  isConfigSupported(config: VideoDecoderConfig): Promise<VideoDecoderSupport>;
}
interface VideoDecoderInit {
  output: VideoFrameOutputCallback;
  error: WebCodecsErrorCallback;
}
type VideoFrameOutputCallback = (output: VideoFrame) => void;
interface AudioEncoder {
  new(init: AudioEncoderInit);
  readonly state: CodecState;
  readonly encodeQueueSize: number;
  configure(config: AudioEncoderConfig): void;
  encode(data: AudioData): void;
  flush(): Promise<void>;
  reset(): void;
  close(): void;
  isConfigSupported(config: AudioEncoderConfig): Promise<AudioEncoderSupport>;
}
interface AudioEncoderInit {
  output: EncodedAudioChunkOutputCallback;
  error: WebCodecsErrorCallback;
}
type EncodedAudioChunkOutputCallback = (
  output: EncodedAudioChunk,
  metadata?: EncodedAudioChunkMetadata
) => void;
interface EncodedAudioChunkMetadata {
  decoderConfig?: AudioDecoderConfig;
}
interface VideoEncoder {
  new(init: VideoEncoderInit);
  readonly state: CodecState;
  readonly encodeQueueSize: number;
  configure(config: VideoEncoderConfig): void;
  encode(frame: VideoFrame, options?: VideoEncoderEncodeOptions): void;
  flush(): Promise<void>;
  reset(): void;
  close(): void;
  isConfigSupported(config: VideoEncoderConfig): Promise<boolean>;
}
interface VideoEncoderInit {
  output: EncodedVideoChunkOutputCallback;
  error: WebCodecsErrorCallback;
}
type EncodedVideoChunkOutputCallback = (
  chunk: EncodedVideoChunk,
  metadata?: EncodedVideoChunkMetadata
) => void;
interface EncodedVideoChunkMetadata {
  decoderConfig?: VideoDecoderConfig;
  svc?: SvcOutputMetadata;
  alphaSideData?: BufferSource;
}
interface SvcOutputMetadata {
  temporalLayerId?: number;
}
interface AudioDecoderSupport {
  supported?: boolean;
  config?: AudioDecoderConfig;
}
interface VideoDecoderSupport {
  supported?: boolean;
  config?: VideoDecoderConfig;
}
interface AudioEncoderSupport {
  supported?: boolean;
  config?: AudioEncoderConfig;
}
interface VideoEncoderSupport {
  supported?: boolean;
  config?: VideoEncoderConfig;
}
interface AudioDecoderConfig {
  codec: string;
  sampleRate: number;
  numberOfChannels: number;
  description?: BufferSource;
}
interface VideoDecoderConfig {
  codec: string;
  description?: BufferSource;
  codedWidth?: number;
  codedHeight?: number;
  displayAspectWidth?: number;
  displayAspectHeight?: number;
  colorSpace?: VideoColorSpaceInit;
  hardwareAcceleration?: HardwareAcceleration;
  optimizeForLatency?: boolean;
}
interface AudioEncoderConfig {
  codec: string;
  sampleRate?: number;
  numberOfChannels?: number;
  bitrate?: number;
}
interface VideoEncoderConfig {
  codec: string;
  width: number;
  height: number;
  displayWidth?: number;
  displayHeight?: number;
  bitrate?: number;
  framerate?: number;
  hardwareAcceleration?: HardwareAcceleration;
  alpha?: AlphaOption;
  scalabilityMode?: string;
  bitrateMode?: BitrateMode;
  latencyMode?: LatencyMode;
  avc?: {
    format: "avc" | "annexb";
  };
}
type HardwareAcceleration = 'no-preference' | 'prefer-hardware' | 'prefer-software';
type AlphaOption = 'keep' | 'discard';
type LatencyMode = 'quality' | 'realtime';
interface VideoEncoderEncodeOptions {
  keyFrame?: boolean;
}
type CodecState = 'unconfigured' | 'configured' | 'closed';
type WebCodecsErrorCallback = (error: DOMException) => void;
interface EncodedAudioChunk {
  new(init: EncodedAudioChunkInit);
  readonly type: EncodedAudioChunkType;
  readonly timestamp: number;
  readonly duration: number;
  readonly byteLength: number;
  copyTo(destination: BufferSource): void;
}
interface EncodedAudioChunkInit {
  type: EncodedAudioChunkType;
  timestamp: number;
  duration?: number;
  data: BufferSource;
}
type EncodedAudioChunkType = 'key' | 'delta';
interface EncodedVideoChunk {
  new(init: EncodedVideoChunkInit);
  readonly type: EncodedVideoChunkType;
  readonly timestamp: number;
  readonly duration: number;
  readonly byteLength: number;
  copyTo(destination: BufferSource): void;
}
interface EncodedVideoChunkInit {
  type: EncodedVideoChunkType;
  timestamp: number;
  duration?: number;
  data: BufferSource;
}
type EncodedVideoChunkType = 'key' | 'delta';
interface AudioData {
  new(init: AudioDataInit);
  readonly format: AudioSampleFormat;
  readonly sampleRate: number;
  readonly numberOfFrames: number;
  readonly numberOfChannels: number;
  readonly duration: number;
  readonly timestamp: number;
  allocationSize(options: AudioDataCopyToOptions): number;
  copyTo(destination: BufferSource, options: AudioDataCopyToOptions): void;
  clone(): AudioData;
  close(): void;
}
declare var AudioData: {
  prototype: AudioData;
  new(init: AudioDataInit): AudioData;
};
interface AudioDataInit {
  format: AudioSampleFormat;
  sampleRate: number;
  numberOfFrames: number;
  numberOfChannels: number;
  timestamp: number;
  data: BufferSource;
}
interface AudioDataCopyToOptions {
  planeIndex: number;
  frameOffset?: number;
  frameCount?: number;
  format?: AudioSampleFormat;
}
type AudioSampleFormat =
  | 'u8'
  | 's16'
  | 's32'
  | 'f32'
  | 'u8-planar'
  | 's16-planar'
  | 's32-planar'
  | 'f32-planar';
interface VideoFrame {
  new(image: CanvasImageSource, init?: VideoFrameInit);
  new(data: BufferSource, init: VideoFrameBufferInit);
  readonly format: VideoPixelFormat;
  readonly codedWidth: number;
  readonly codedHeight: number;
  readonly codedRect: DOMRectReadOnly;
  readonly visibleRect: DOMRectReadOnly;
  readonly displayWidth: number;
  readonly displayHeight: number;
  readonly duration: number;
  readonly timestamp: number;
  readonly colorSpace: VideoColorSpace;
  allocationSize(options?: VideoFrameCopyToOptions): number;
  copyTo(destination: BufferSource, options?: VideoFrameCopyToOptions): Promise<Array<PlaneLayout>>;
  clone(): VideoFrame;
  close(): void;
}
interface VideoFrameInit {
  duration?: number;
  timestamp?: number;
  alpha?: AlphaOption;
  visibleRect?: DOMRectInit;
  displayWidth?: number;
  displayHeight?: number;
}
interface VideoFrameBufferInit {
  format: VideoPixelFormat;
  codedWidth: number;
  codedHeight: number;
  timestamp: number;
  duration?: number;
  layout?: Array<PlaneLayout>;
  visibleRect?: DOMRectInit;
  displayWidth?: number;
  displayHeight?: number;
  colorSpace?: VideoColorSpaceInit;
}
interface VideoFrameCopyToOptions {
  rect?: DOMRectInit;
  layout?: Array<PlaneLayout>;
}
interface PlaneLayout {
  offset: number;
  stride: number;
}
type VideoPixelFormat =
  | 'I420'
  | 'I420A'
  | 'I422'
  | 'I444'
  | 'NV12'
  | 'RGBA'
  | 'RGBX'
  | 'BGRA'
  | 'BGRX';
interface VideoColorSpace {
  new(init?: VideoColorSpaceInit);
  readonly primaries: VideoColorPrimaries;
  readonly transfer: VideoTransferCharacteristics;
  readonly matrix: VideoMatrixCoefficients;
  readonly fullRange: boolean;
  toJSON(): VideoColorSpaceInit;
}
interface VideoColorSpaceInit {
  primaries?: VideoColorPrimaries;
  transfer?: VideoTransferCharacteristics;
  matrix?: VideoMatrixCoefficients;
  fullRange?: boolean;
}
type VideoColorPrimaries = 'bt709' | 'bt470bg' | 'smpte170m';
type VideoTransferCharacteristics = 'bt709' | 'smpte170m' | 'iec61966-2-1';
type VideoMatrixCoefficients = 'rgb' | 'bt709' | 'bt470bg' | 'smpte170m';
interface ImageDecoder {
  new(init: ImageDecoderInit);
  readonly complete: boolean;
  readonly completed: Promise<void>;
  readonly tracks: ImageTrackList;
  decode(options?: ImageDecodeOptions): Promise<ImageDecodeResult>;
  reset(): void;
  close(): void;
  isTypeSupported(type: string): Promise<boolean>;
}
type ImageBufferSource = BufferSource | ReadableStream;
interface ImageDecoderInit {
  type: string;
  data: ImageBufferSource;
  premultiplyAlpha?: PremultiplyAlpha;
  colorSpaceConversion?: ColorSpaceConversion;
  desiredWidth?: number;
  desiredHeight?: number;
  preferAnimation?: boolean;
}
interface ImageDecodeOptions {
  frameIndex?: number;
  completeFramesOnly?: boolean;
}
interface ImageDecodeResult {
  image: VideoFrame;
  complete: boolean;
}
interface ImageTrackList {
  (index: number): ImageTrack;
  readonly ready: Promise<void>;
  readonly length: number;
  readonly selectedIndex: number;
  readonly selectedTrack: ImageTrack;
}
interface ImageTrack extends EventTarget {
  readonly animated: boolean;
  readonly frameCount: number;
  readonly repetitionCount: number;
  onchange: EventHandler;
  selected: boolean;
}
