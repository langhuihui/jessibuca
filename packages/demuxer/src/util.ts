export const samplingFrequencyIndexMap = [
  96000,
  88200,
  64000,
  48000,
  44100,
  32000,
  24000,
  22050,
  16000,
  12000,
  11025,
  8000,
  7350,
  -1, // reserved
  -1, // reserved
  -1, // reserved
];
export function adtsToAsc(adts: Uint8Array) {
  const profile = ((adts[2] & 0xc0) >>> 6) + 1;
  const samplingFrequencyIndex = (adts[2] & 0x3c) >>> 2;
  const channelConfiguration = ((adts[2] & 0x01) << 2) | ((adts[3] & 0xc0) >>> 6);
  const audioSpecificConfig = new Uint8Array([
    ((profile & 0x03) << 3) | ((samplingFrequencyIndex & 0x0e) >> 1),
    ((samplingFrequencyIndex & 0x01) << 7) | ((channelConfiguration & 0x0f) << 3),
  ]);
  return {
    profile,
    sampleRate: samplingFrequencyIndexMap[samplingFrequencyIndex],
    channel: channelConfiguration,
    audioSpecificConfig
  };
}