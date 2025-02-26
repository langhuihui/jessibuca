interface Decryptor {
  decrypt(data: Uint8Array, key: Uint8Array, iv: Uint8Array): Promise<Uint8Array>;
}

export interface HlsOption {
  isLive?: boolean;
  softDecode?: boolean;
  targetLatency?: number;
  maxPlaylistSize?: number;
  maxLatency?: number;
  bufferBehind?: number;
  maxJumpDistance?: number;
  startTime?: number;
  retryCount?: number;
  retryDelay?: number;
  loadTimeout?: number;
  preloadTime?: number;
  /**
   * disconnectTime 是当直播暂停后，直播延迟超过该值就会断流，作为插件使用时该值是 0 秒，用户暂停直播后就会断流，点击播放会重新拉流。
   * 但是用户频繁的暂停播放，就会导致频繁的断流和拉流，设置该值可以让短时间的停止播放不断流。
   * 特有：直播断流时间，默认 0 秒，（独立使用时等于 maxLatency）
   */
  disconnectTime?: number;
  fetchOptions?: RequestInit;
  onPreM3U8Parse?: (m3u8: string) => string | void;
  decryptor?: Decryptor;
  pollRetryCount?: number;
  allowedStreamTrackChange?: boolean;
}

export function getConfig(cfg: Partial<HlsOption>): HlsOption {
  return {
    isLive: true,
    maxPlaylistSize: 50,
    retryCount: 3, //  重试 3 次，默认值
    retryDelay: 1000, // 每次重试间隔 1 秒，默认值
    pollRetryCount: 2,
    loadTimeout: 10000, // 请求超时时间为 10 秒，默认值
    preloadTime: 30, // preloadTime指定允许的预加载buffer的最大长度(单位s)
    softDecode: false,
    bufferBehind: 10,
    maxJumpDistance: 3,
    startTime: 0,
    // targetLatency 和 maxLatency 关系是，当直播延迟超过 maxLatency 时，hls 就会将当前播放时间点跳转到 targetLatency 位置。
    // 所以配置这两个值的时候需要确保 maxLatency 大于 targetLatency，并且应该大很多，默认值是大两倍。
    targetLatency: 10, // 特有：直播目标延迟，默认 10 秒
    maxLatency: 20, // 特有：直播允许的最大延迟，默认 20 秒
    allowedStreamTrackChange: true,
    ...cfg,
  };
} 