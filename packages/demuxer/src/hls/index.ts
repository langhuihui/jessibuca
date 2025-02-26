import { getConfig, HlsOption } from "./config";
import ManifestLoader from "./manifest-loader";
// import SegmentLoader from "./segment-loader";
import Playlist from "./playlist";
import BufferService from "./buffer-service";
import SeiService from "./utils/sei";
import MediaStatsService from "./media-stats-service";
import { StreamingError, ERR } from "./error";
import EventEmitter from "eventemitter3";

interface Stats {
  // Add stats interface properties based on MediaStatsService
  [key: string]: any;
}

interface SpeedInfo {
  // Add speed info interface properties based on SegmentLoader
  [key: string]: any;
}

interface SwitchUrlOptions {
  bitrate?: number;
  [key: string]: any;
}

interface Stream {
  url?: string;
  bitrate: number;
  subtitleStreams?: any[];
  totalDuration: number;
}

interface Segment {
  start: number;
}

interface PlaylistManifest {
  isMaster: boolean;
}

export default class HlsLoader extends EventEmitter {
  public config: HlsOption | null;
  public canVideoPlay: boolean;
  public $videoElement: HTMLVideoElement | null;

  private _manifestLoader: ManifestLoader | null;
  // private _segmentLoader: SegmentLoader | null;
  private _playlist: Playlist | null;
  private _bufferService: BufferService | null;
  private _seiService: SeiService | null;
  private _stats: MediaStatsService | null;
  private _prevSegSn: number | null;
  private _prevSegCc: number | null;
  private _tickTimer: NodeJS.Timeout | null;
  private _tickInterval: number;
  private _segmentProcessing: boolean;
  private _reloadOnPlay: boolean;
  private _switchUrlOpts: SwitchUrlOptions | null;
  private _disconnectTimer: NodeJS.Timeout | null;
  private _urlSwitching?: boolean;

  constructor(cfg: Partial<HlsOption> = {}) {
    super();
    this.config = null;
    this._manifestLoader = null;
    // this._segmentLoader = null;
    this._playlist = null;
    this._bufferService = null;
    this._seiService = null;
    this._stats = null;
    this._prevSegSn = null;
    this._prevSegCc = null;
    this._tickTimer = null;
    this._tickInterval = 500;
    this._segmentProcessing = false;
    this._reloadOnPlay = false;
    this._switchUrlOpts = null;
    this._disconnectTimer = null;

    this.canVideoPlay = false;
    this.$videoElement = null;
    this.config = getConfig(cfg);
    this._manifestLoader = new ManifestLoader(this);
    // this._segmentLoader = new SegmentLoader(this);
    this._playlist = new Playlist(this);
    this._bufferService = new BufferService(this);
    this._seiService = new SeiService(this);
    this._stats = new MediaStatsService(this, 90000);
  }

  public async destroy(): Promise<void> {
    this._playlist?.reset();
    this._segmentLoader?.reset();
    this._seiService?.reset();
    await Promise.all([this._clear(), this._bufferService?.destroy() || Promise.resolve()]);

    if (this._manifestLoader) {
      await this._manifestLoader.destroy();
      this._manifestLoader = null;
    }

    if (this._segmentLoader) {
      this._segmentLoader.destroy();
      this._segmentLoader = null;
    }
    if (this._playlist) {
      this._playlist.destroy();
      this._playlist = null;
    }
  }

  private _startTick(): void {
    this._stopTick();
    this._tickTimer = setTimeout(() => {
      this._tick();
    }, this._tickInterval);
  }

  private _stopTick(): void {
    if (this._tickTimer) {
      clearTimeout(this._tickTimer);
    }
    this._tickTimer = null;
  }

  private _tick(): void {
    this._startTick();
    this._loadSegment();
  }

  public get isLive(): boolean {
    return this._playlist?.isLive || false;
  }

  public get streams(): any[] {
    return this._playlist?.streams || [];
  }

  public get currentStream(): Stream | null {
    return this._playlist?.currentStream || null;
  }

  public get hasSubtitle(): boolean {
    return this._playlist?.hasSubtitle || false;
  }

  public get baseDts(): number | undefined {
    return this._bufferService?.baseDts;
  }

  public speedInfo(): SpeedInfo {
    return this._segmentLoader?.speedInfo() || {};
  }

  public resetBandwidth(): void {
    this._segmentLoader?.resetBandwidth();
  }

  public getStats(): Stats {
    return this._stats?.getStats() || {};
  }

  public async loadSource(url: string): Promise<boolean> {
    await this._reset();
    await this._loadData(url);
    this._startTick();
    return true;
  }

  private async _loadData(url: string): Promise<void> {
    try {
      if (url) url = url.trim();
    } catch (e) {
      // Ignore trim errors
    }

    if (!url) {
      throw this._emitError(new StreamingError(ERR.OTHER, ERR.OTHER, null, null, 'm3u8 url is missing'));
    }

    const manifest = await this._loadM3U8(url);
    const currentStream = this.currentStream;

    if (this._urlSwitching) {
      if (currentStream?.bitrate === 0 && this._switchUrlOpts?.bitrate) {
        currentStream.bitrate = this._switchUrlOpts.bitrate;
      }

      const switchTimePoint = this._getSeamlessSwitchPoint();
      if (this.config) {
        this.config.startTime = switchTimePoint;
      }

      const segIdx = this._playlist?.findSegmentIndexByTime(switchTimePoint);
      const nextSeg = this._playlist?.getSegmentByIndex(segIdx + 1);

      if (nextSeg) {
        // move to next segment in case of media stall
        const bufferClearStartPoint = nextSeg.start;
        // await this._bufferService.removeBuffer(bufferClearStartPoint)
      }
    }

    if (manifest) {
      if (this.isLive) {
        this._bufferService?.setLiveSeekableRange(0, 0xffffffff);

        if (this.config?.targetLatency && this._playlist?.totalDuration &&
          this.config.targetLatency < this._playlist.totalDuration) {
          this.config.targetLatency = this._playlist.totalDuration;
          this.config.maxLatency = 1.5 * this.config.targetLatency;
        }

        if (!manifest.isMaster) {
          this._pollM3U8(url);
        }
      } else {
        await this._bufferService?.updateDuration(currentStream?.totalDuration || 0);
      }
    }
    await this._loadSegment();
  }

  private async _loadM3U8(url: string): Promise<PlaylistManifest | undefined> {
    let playlist: PlaylistManifest | undefined;
    try {
      [playlist] = await this._manifestLoader?.load(url) || [];
    } catch (error) {
      throw this._emitError(StreamingError.create(error));
    }
    if (!playlist) {
      return;
    }
    this._playlist?.upsertPlaylist(playlist);

    if (playlist.isMaster) {
      if (this._playlist?.currentStream?.subtitleStreams?.length) {
        this.emit(HLS_EVENTS.SUBTITLE_PLAYLIST, {
          list: this._playlist.currentStream.subtitleStreams
        });
      }
      await this._refreshM3U8();
    }
    this.emit(HLS_EVENTS.STREAM_PARSED);
    return playlist;
  }

  private _refreshM3U8(): void {
    const stream = this._playlist?.currentStream;
    if (!stream || !stream.url) {
      throw this._emitError(StreamingError.create(null, null, new Error('m3u8 url is not defined')));
    }
  }

  private async _pollM3U8(url: string, audioUrl?: string, subtitleUrl?: string): Promise<void> {
    if (!this.isLive) return;

    try {
      const [playlist] = await this._manifestLoader?.load(url) || [];
      if (playlist) {
        this._playlist?.upsertPlaylist(playlist);
      }
    } catch (error) {
      if (this._playlist?.retryCount < (this.config?.pollRetryCount || 2)) {
        this._playlist?.increaseRetryCount();
        setTimeout(() => {
          this._pollM3U8(url, audioUrl, subtitleUrl);
        }, this.config?.retryDelay || 1000);
      } else {
        this._emitError(StreamingError.create(error));
      }
    }
  }

  private _loadSegment = async (): Promise<void> => {
    if (this._segmentProcessing) return;

    try {
      this._segmentProcessing = true;
      await this._loadSegmentDirect();
    } catch (error) {
      this._emitError(StreamingError.create(error));
    } finally {
      this._segmentProcessing = false;
    }
  };

  private async _loadSegmentDirect(): Promise<void> {
    const currentStream = this._playlist?.currentStream;
    if (!currentStream) return;

    const seg = this._playlist?.getNextSegment();
    if (!seg) return;

    let audioSeg;
    if (currentStream.audioStream) {
      audioSeg = this._playlist?.getNextAudioSegment();
      if (!audioSeg) return;
    }

    if (this._prevSegSn !== null) {
      const discontinuity = seg.sn - this._prevSegSn !== 1;
      if (discontinuity && this._prevSegCc === seg.cc) {
        this.emit(HLS_EVENTS.DISCONTINUITY, {
          prev: this._prevSegSn,
          current: seg.sn
        });
      }
    }

    this._prevSegSn = seg.sn;
    this._prevSegCc = seg.cc;

    await this._reqAndBufferSegment(seg, audioSeg);
  }

  private async _reqAndBufferSegment(seg: any, audioSeg?: any): Promise<void> {
    try {
      const [data, audioData] = await this._segmentLoader?.load(seg, audioSeg) || [];
      if (!data) return;

      const bufferData = await this._bufferService?.bufferSegment(
        data,
        audioData,
        seg,
        audioSeg
      );

      if (bufferData) {
        this.emit(HLS_EVENTS.BUFFER_APPENDED, bufferData);
      }

      if (this._playlist?.isEndList && this._playlist?.isComplete) {
        this._end();
      }
    } catch (error) {
      throw error;
    }
  }

  private async _clear(): Promise<void> {
    this._stopTick();
    if (this._disconnectTimer) {
      clearTimeout(this._disconnectTimer);
      this._disconnectTimer = null;
    }
    await this._bufferService?.clear();
  }

  private async _reset(reuseMse = false): Promise<void> {
    this._prevSegSn = null;
    this._prevSegCc = null;
    this._segmentProcessing = false;
    this._reloadOnPlay = false;
    this._switchUrlOpts = null;

    await this._clear();
    this._playlist?.reset();
    this._segmentLoader?.reset();
    this._seiService?.reset();
    if (!reuseMse) {
      await this._bufferService?.destroy();
    }
  }

  private _end(): void {
    this._stopTick();
    this.emit(HLS_EVENTS.ENDED);
  }

  private _emitError(error: StreamingError, endOfStream = false): StreamingError {
    if (error.errorType === ERR.NETWORK) {
      this._reloadOnPlay = true;
    }

    if (endOfStream) {
      this._end();
    } else {
      this._stopTick();
    }

    this.emit(HLS_EVENTS.ERROR, error);
    return error;
  }

  private _getSeamlessSwitchPoint(): number {
    const currentTime = this.$videoElement?.currentTime || 0;
    const buffered = this._bufferService?.getBuffered();
    if (!buffered?.length) return currentTime;

    for (let i = 0; i < buffered.length; i++) {
      const start = buffered.start(i);
      const end = buffered.end(i);
      if (currentTime >= start && currentTime <= end) {
        return end;
      }
    }
    return currentTime;
  }

  public getDemuxBufferedDuration(): number {
    return this._bufferService?.getDemuxBufferedDuration() || 0;
  }

  public getDemuxBufferedListLength(): number {
    return this._bufferService?.getDemuxBufferedListLength() || 0;
  }

  public getDemuxAudioBufferedListLength(): number {
    return this._bufferService?.getDemuxAudioBufferedListLength() || 0;
  }

  public getDemuxVideoBufferedListLength(): number {
    return this._bufferService?.getDemuxVideoBufferedListLength() || 0;
  }
} 