import NetLoader from "../../utils/netLoader";
import { StreamingError, ERR } from "../error";
import M3U8Parser from "./parser";
import { HLS_EVENTS } from '../events';
import { HlsLoader } from "../index";

interface LoaderResponse {
  data: string;
}

interface Playlist {
  live?: boolean;
  segments?: any[];
  isMaster?: boolean;
}

export default class ManifestLoader {
  private hls: HlsLoader;
  private player: any;
  private readonly TAG_NAME: string;
  private _timer: NodeJS.Timeout | null;
  private _loader: NetLoader | null;
  private _audioLoader: NetLoader | null;
  private _subtitleLoader: NetLoader | null;

  constructor(hls: HlsLoader) {
    this.hls = hls;
    this.player = hls.player;
    this.TAG_NAME = 'HlsManifestLoader';
    this._timer = null;

    const { retryCount, retryDelay, loadTimeout, fetchOptions } = this.hls.config || {};

    const loaderConfig = {
      ...fetchOptions,
      responseType: 'text' as const,
      retry: retryCount,
      retryDelay: retryDelay,
      timeout: loadTimeout,
      onRetryError: this._onLoaderRetry
    };

    this._loader = new NetLoader(loaderConfig, this.player);
    this._audioLoader = new NetLoader(loaderConfig, this.player);
    this._subtitleLoader = new NetLoader(loaderConfig, this.player);
  }

  public async destroy(): Promise<void> {
    await this.stopPoll();
    if (this._audioLoader) {
      this._audioLoader.destroy();
      this._audioLoader = null;
    }

    if (this._subtitleLoader) {
      this._subtitleLoader.destroy();
      this._subtitleLoader = null;
    }

    if (this._loader) {
      this._loader.destroy();
      this._loader = null;
    }
  }

  public async load(
    url: string,
    audioUrl?: string,
    subtitleUrl?: string
  ): Promise<[Playlist?, Playlist?, Playlist?]> {
    this.player.debug.log(this.TAG_NAME, 'load()', url, audioUrl, subtitleUrl);

    if (!this._loader) {
      throw new Error('Loader not initialized');
    }

    const toLoad: Promise<LoaderResponse | undefined>[] = [this._loader.load(url)];

    if (audioUrl && this._audioLoader) {
      toLoad.push(this._audioLoader.load(audioUrl));
    }

    if (subtitleUrl && this._subtitleLoader) {
      toLoad.push(this._subtitleLoader.load(subtitleUrl));
    }

    let videoText: string;
    let audioText: string | undefined;
    let subtitleText: string | undefined;

    try {
      const [video, audio, subtitle] = await Promise.all(toLoad);

      if (!video) return [];

      videoText = video.data;

      if (audioUrl) {
        audioText = audio?.data;
        subtitleText = subtitle?.data;
      } else {
        subtitleText = audio?.data;
      }
    } catch (error) {
      throw StreamingError.network(error);
    }

    let playlist: Playlist | undefined;
    let audioPlaylist: Playlist | undefined;
    let subtitlePlaylist: Playlist | undefined;

    try {
      playlist = M3U8Parser.parse(videoText, url);

      if (playlist?.live === false && playlist.segments && !playlist.segments.length) {
        throw new Error('empty segments list');
      }

      if (audioText) {
        audioPlaylist = M3U8Parser.parse(audioText, audioUrl);
      }
      if (subtitleText) {
        subtitlePlaylist = M3U8Parser.parse(subtitleText, subtitleUrl);
      }
    } catch (error) {
      throw new StreamingError(ERR.MANIFEST, ERR.HLS, error);
    }

    if (playlist) {
      if (playlist.isMaster) {
        // hls manifest loaded
        // 主从m3u8格式时，master m3u8文件加载并解析完成后, 抛出master m3u8解析后的结构
        this.hls.emit(HLS_EVENTS.HLS_MANIFEST_LOADED, { playlist });
      } else {
        // hls level loaded
        // 二级m3u8加载并解析完成后，抛出解析后的结构，
        this.hls.emit(HLS_EVENTS.HLS_LEVEL_LOADED, { playlist });
      }
    }

    return [playlist, audioPlaylist, subtitlePlaylist];
  }

  public poll(
    url: string,
    audioUrl: string | undefined,
    subtitleUrl: string | undefined,
    cb: (playlist: Playlist, audioPlaylist?: Playlist, subtitlePlaylist?: Playlist) => void,
    errorCb: (error: Error) => void,
    time?: number
  ): void {
    if (this._timer) {
      clearTimeout(this._timer);
    }

    time = time || 3000;

    let retryCount = this.hls.config?.pollRetryCount || 0;

    const fn = async (): Promise<void> => {
      if (this._timer) {
        clearTimeout(this._timer);
      }
      try {
        const res = await this.load(url, audioUrl, subtitleUrl);
        if (!res[0]) return;
        retryCount = this.hls.config?.pollRetryCount || 0;
        cb(res[0], res[1], res[2]);
      } catch (e) {
        retryCount--;
        if (retryCount <= 0) {
          errorCb(e instanceof Error ? e : new Error(String(e)));
        }
      }
      this._timer = setTimeout(fn, time);
    };
    this._timer = setTimeout(fn, time);
  }

  public async stopPoll(): Promise<void> {
    if (this._timer) {
      clearTimeout(this._timer);
    }
    return this.cancel();
  }

  public async cancel(): Promise<void> {
    await Promise.all([
      this._loader?.cancel(),
      this._audioLoader?.cancel()
    ]);
  }

  private _onLoaderRetry = (error: Error, retryTime: number): void => {
    // 请求发生重试时触发。参数如下。
    this.hls.emit(HLS_EVENTS.LOAD_RETRY, {
      error: StreamingError.network(error),
      retryTime
    });
  };
} 