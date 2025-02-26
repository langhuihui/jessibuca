import NetLoader from "../../utils/netLoader";
import { BandwidthService } from "../../utils/bandwidthService";
import { HLS_EVENTS } from '../events';
import { StreamingError } from "../error";
import { default as HlsLoader } from "../index";

interface MediaSegmentKey {
  url?: string;
  iv?: string;
}

interface InitSegment {
  url: string;
  key?: MediaSegmentKey;
}

interface MediaSegment {
  url: string;
  initSegment?: InitSegment;
  key?: MediaSegmentKey;
}

interface LoaderResponse {
  data: ArrayBuffer;
  response: Response;
  option?: {
    firstByteTime: number;
    startTime: number;
    endTime: number;
    contentLength?: number;
  };
}

interface SegmentData {
  data: ArrayBuffer;
  map?: ArrayBuffer;
  key?: ArrayBuffer;
  mapKey?: ArrayBuffer;
  keyIv?: string;
  mapKeyIv?: string;
}

export default class SegmentLoader {
  private hls: HlsLoader;
  private player: any;
  private _bandwidthService: BandwidthService;
  private _segmentLoader: NetLoader;
  private _audioSegmentLoader: NetLoader;
  private _keyLoader: NetLoader;
  private _mapCache: { [key: string]: ArrayBuffer; };
  private _keyCache: { [key: string]: ArrayBuffer; };
  public error: Error | null;

  constructor(hls: HlsLoader) {
    this.hls = hls;
    this.player = hls.player;
    this._bandwidthService = new BandwidthService();
    this._mapCache = {};
    this._keyCache = {};
    this.error = null;

    const { retryCount, retryDelay, loadTimeout, fetchOptions } = this.hls.config || {};
    const loaderConfig = {
      ...fetchOptions,
      responseType: 'arraybuffer' as const,
      retry: retryCount,
      retryDelay: retryDelay,
      timeout: loadTimeout,
      onRetryError: this._onLoaderRetry
    };

    this._segmentLoader = new NetLoader(loaderConfig, this.player);
    this._audioSegmentLoader = new NetLoader(loaderConfig, this.player);
    this._keyLoader = new NetLoader(loaderConfig, this.player);
  }

  public destroy(): void {
    this.reset();
    if (this._keyLoader) {
      this._keyLoader.destroy();
      this._keyLoader = null;
    }
    if (this._audioSegmentLoader) {
      this._audioSegmentLoader.destroy();
      this._audioSegmentLoader = null;
    }
    if (this._segmentLoader) {
      this._segmentLoader.destroy();
      this._segmentLoader = null;
    }
  }

  public speedInfo(): { speed: number; avgSpeed: number; } {
    return {
      speed: this._bandwidthService.getLatestSpeed(),
      avgSpeed: this._bandwidthService.getAvgSpeed()
    };
  }

  public resetBandwidth(): void {
    this._bandwidthService.reset();
  }

  public load(
    seg?: MediaSegment,
    audioSeg?: MediaSegment,
    loadInit?: boolean,
    loadAudioInit: boolean = loadInit
  ): Promise<[SegmentData?, SegmentData?]> {
    const toLoad: Promise<SegmentData | undefined>[] = [];
    if (seg) toLoad[0] = this.loadVideoSegment(seg, loadInit);
    if (audioSeg) toLoad[1] = this.loadAudioSegment(audioSeg, loadAudioInit);
    return Promise.all(toLoad);
  }

  public loadVideoSegment(seg: MediaSegment, loadInit?: boolean): Promise<SegmentData | undefined> {
    return this._loadSegment(this._segmentLoader, seg, loadInit);
  }

  public loadAudioSegment(seg: MediaSegment, loadInit?: boolean): Promise<SegmentData | undefined> {
    return this._loadSegment(this._audioSegmentLoader, seg, loadInit);
  }

  private async _loadSegment(
    segLoader: NetLoader,
    seg: MediaSegment,
    loadInit?: boolean
  ): Promise<SegmentData | undefined> {
    let map: ArrayBuffer | undefined;
    let key: ArrayBuffer | undefined;
    let keyIv: string | undefined;
    let mapKey: ArrayBuffer | undefined;
    let mapKeyIv: string | undefined;
    const toLoad: Promise<void>[] = [];

    // load start
    // 开始拉流或者后续播放阶段时获取
    // 分片在发送请求之前触发，参数为 { url: string }，url 为请求 url 。
    this.hls.emit(HLS_EVENTS.LOAD_START, { url: seg.url });

    const segmentPromise = segLoader.load(seg.url);
    toLoad[0] = segmentPromise;

    if (loadInit && seg.initSegment) {
      const mapUrl = seg.initSegment.url;
      map = this._mapCache[mapUrl];

      if (!map) {
        this.hls.emit(HLS_EVENTS.LOAD_START, { url: mapUrl });

        toLoad[1] = segLoader.load(mapUrl).then(r => {
          if (r) {
            const l = Object.keys(this._mapCache);
            if (l.length > 30) this._mapCache = {};
            map = this._mapCache[mapUrl] = r.data;
            this._emitOnLoaded(r, mapUrl);
          }
        });
      }

      const keyUrl = seg.initSegment.key?.url;

      if (keyUrl) {
        mapKeyIv = seg.initSegment.key?.iv;
        mapKey = this._keyCache[keyUrl];

        if (!mapKey) {
          this.hls.emit(HLS_EVENTS.LOAD_START, { url: keyUrl });
          toLoad[2] = this._keyLoader.load(keyUrl).then(r => {
            if (r) {
              mapKey = this._keyCache[keyUrl] = r.data;
              this._emitOnLoaded(r, keyUrl);
            }
          });
        }
      }
    }

    const keyUrl = seg.key?.url;

    if (keyUrl) {
      keyIv = seg.key?.iv;
      key = this._keyCache[keyUrl];
      if (!key) {
        this.hls.emit(HLS_EVENTS.LOAD_START, { url: keyUrl });
        toLoad[3] = this._keyLoader.load(keyUrl).then(r => {
          if (r) {
            key = this._keyCache[keyUrl] = r.data;
            this._emitOnLoaded(r, keyUrl);
          }
        });
      }
    }

    const [s] = await Promise.all(toLoad);
    if (!s) return;
    const data = s.data;
    this._emitOnLoaded(s, seg.url);

    return {
      data,
      map,
      key,
      mapKey,
      keyIv,
      mapKeyIv
    };
  }

  public reset(): void {
    this.error = null;
    this._mapCache = {};
    this._keyCache = {};
    this._bandwidthService.reset();
  }

  public async cancel(): Promise<void> {
    await Promise.all([
      this._keyLoader?.cancel(),
      this._segmentLoader?.cancel(),
      this._audioSegmentLoader?.cancel()
    ]);
  }

  private _emitOnLoaded = (res: LoaderResponse, url: string): void => {
    const { data, response, option } = res;
    const { firstByteTime, startTime, endTime, contentLength } = option || {};
    const time = endTime - startTime;

    this._bandwidthService.addRecord(contentLength || data.byteLength, time);

    // emit speed 事件
    // 当收集到网络速度统计时触发，参数如下
    this.hls.emit(HLS_EVENTS.SPEED, { time, byteLength: contentLength, url });
    // 在请求完成后触发，参数为 { url: string }，url 为请求 url 。
    this.hls.emit(HLS_EVENTS.LOAD_COMPLETE, { url, elapsed: time || 0 });

    this.hls.emit(HLS_EVENTS.TTFB, {
      url,
      responseUrl: response.url,
      elapsed: firstByteTime - startTime
    });
    // 接收到请求响应头时触发，参数为 { headers: Headers | Recored<string, string>} 。
    // 如果当前环境支持 fetch 则 headers 为 Response.headers，否则是普通对象。
    this.hls.emit(HLS_EVENTS.LOAD_RESPONSE_HEADERS, { headers: response.headers });
  };

  private _onLoaderRetry = (error: Error, retryTime: number): void => {
    this.hls.emit(HLS_EVENTS.LOAD_RETRY, {
      error: StreamingError.network(error),
      retryTime
    });
  };
} 