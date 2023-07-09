type VideoType = 'avc' | 'hevc' | 'unknown';
interface VideoDecoderConfig {
  codec: string;
  videoType: VideoType,
  extraData?: BufferSource,
  avc?: {
    format: "avcc" | "annexb";
  },
  hevc?: {
    format: "hvcc" | "annexb";
  };
};

interface AudioDecoderConfig {
  codec: AudioCodec,
  extraData?: BufferSource,
  outSampleType?: AudioSampleFormat,
  sampleRate: number,
  numberOfChannels: number,
}