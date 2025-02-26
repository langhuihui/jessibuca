import { HLS_EVENTS } from '../events';
import { HlsLoader } from '../index';

interface Sample {
  duration: number;
  gopId: number;
  units: { length: number; }[];
}

interface Track {
  samples: Sample[];
  width?: number;
  height?: number;
  codec?: string;
  codecType?: string;
  fpsNum?: number;
  fpsDen?: number;
  sampleRate?: number;
  channelCount?: number;
}

interface ScriptData {
  data?: {
    onMetaData?: {
      framerate?: number;
    };
  };
}

interface StatsInfo {
  downloadSpeed: number;
  avgSpeed: number;
  currentTime: number;
  bufferEnd: number;
  decodeFps: number;
  encodeType: string;
  audioCodec: string;
  videoCodec: string;
  domain: string;
  fps: number;
  bitrate: number;
  width: number;
  height: number;
  samplerate: number;
  channelCount: number;
  gop: number;
}

class Stats {
  private _timescale: number;
  public encodeType: string;
  public audioCodec: string;
  public videoCodec: string;
  public domain: string;
  public fps: number;
  public bitrate: number;
  public width: number;
  public height: number;
  public samplerate: number;
  public channelCount: number;
  public gop: number;
  private _bitsAccumulateSize: number;
  private _bitsAccumulateDuration: number;

  constructor(timescale: number) {
    this._timescale = timescale;
    this.encodeType = '';
    this.audioCodec = '';
    this.videoCodec = '';
    this.domain = '';
    this.fps = 0;
    this.bitrate = 0;
    this.width = 0;
    this.height = 0;
    this.samplerate = 0;
    this.channelCount = 0;
    this.gop = 0;

    this._bitsAccumulateSize = 0;
    this._bitsAccumulateDuration = 0;
  }

  public getStats(): Omit<StatsInfo, 'downloadSpeed' | 'avgSpeed' | 'currentTime' | 'bufferEnd' | 'decodeFps'> {
    return {
      encodeType: this.encodeType,
      audioCodec: this.audioCodec,
      videoCodec: this.videoCodec,
      domain: this.domain,
      fps: this.fps,
      bitrate: this.bitrate,
      width: this.width,
      height: this.height,
      samplerate: this.samplerate,
      channelCount: this.channelCount,
      gop: this.gop
    };
  }

  public setEncodeType(encode: string): void {
    this.encodeType = encode;
  }

  public setFpsFromScriptData({ data }: ScriptData): void {
    const fps = data?.onMetaData?.framerate;
    if (fps && fps > 0 && fps < 100) {
      this.fps = fps;
    }
  }

  public setVideoMeta(track: Track): void {
    this.width = track.width || 0;
    this.height = track.height || 0;
    this.videoCodec = track.codec || '';
    this.encodeType = track.codecType || '';
    if (track.fpsNum && track.fpsDen) {
      const fps = track.fpsNum / track.fpsDen;
      if (fps > 0 && fps < 100) {
        this.fps = fps;
      }
    }
  }

  public setAudioMeta(track: Track): void {
    this.audioCodec = track.codec || '';
    this.samplerate = track.sampleRate || 0;
    this.channelCount = track.channelCount || 0;
  }

  public setDomain(responseUrl: string): void {
    this.domain = responseUrl.split('/').slice(2, 3)[0];
  }

  public updateBitrate(samples: Sample[]): void {
    if (!this.fps || this.fps >= 100) {
      if (samples.length) {
        const duration = samples.reduce((a, b) => a + b.duration, 0) / samples.length;
        this.fps = Math.round(this._timescale / duration);
      }
    }
    samples.forEach(sample => {
      if (sample.gopId === 1) {
        this.gop++;
      }
      this._bitsAccumulateDuration += sample.duration / (this._timescale / 1000);
      this._bitsAccumulateSize += sample.units.reduce((a, c) => (a + c.length), 0);
      if (this._bitsAccumulateDuration >= 1000) {
        this.bitrate = this._bitsAccumulateSize * 8;
        this._bitsAccumulateDuration = 0;
        this._bitsAccumulateSize = 0;
      }
    });
  }
}

export default class MediaStatsService {
  private _core: HlsLoader;
  private _samples: Sample[];
  private _timescale: number;
  private _stats: Stats;

  constructor(core: HlsLoader, timescale: number = 1000) {
    this._core = core;
    this._timescale = timescale;
    this._stats = new Stats(timescale);
    this._samples = [];
    this._bindEvents();
  }

  public getStats(): StatsInfo {
    const media = this._core?.media || {};
    return {
      ...this._stats.getStats(),
      downloadSpeed: this._core?.speedInfo?.()?.speed || 0,
      avgSpeed: this._core?.speedInfo?.()?.avgSpeed || 0,
      currentTime: media.currentTime || 0,
      bufferEnd: this._core?.bufferInfo?.()?.remaining || 0,
      decodeFps: media.decodeFps || 0
    };
  }

  private _bindEvents(): void {
    this._core.on(HLS_EVENTS.DEMUXED_TRACK, (track: Track) => this._stats.updateBitrate(track.samples));

    this._core.on(HLS_EVENTS.FLV_SCRIPT_DATA, (data: ScriptData) => {
      this._stats.setFpsFromScriptData(data);
    });

    this._core.on(HLS_EVENTS.METADATA_PARSED, (e: { type: string; track: Track; }) => {
      if (e.type === 'video') {
        this._stats.setVideoMeta(e.track);
      } else {
        this._stats.setAudioMeta(e.track);
      }
    });

    this._core.on(HLS_EVENTS.TTFB, (e: { responseUrl: string; }) => {
      this._stats.setDomain(e.responseUrl);
    });
  }

  public reset(): void {
    this._samples = [];
    this._stats = new Stats(this._timescale);
  }
} 