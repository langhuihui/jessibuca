import { clamp } from "../../utils";
import Stream from "./stream";
import { HLS_EVENTS } from '../events';
import { HlsLoader } from "../index";

interface Segment {
  start: number;
  end: number;
  duration: number;
  hasAudio?: boolean;
  hasVideo?: boolean;
}

interface Playlist {
  isMaster?: boolean;
  streams?: any[];
  segments?: Segment[];
}

interface SubtitleSegment {
  sn: number;
  url: string;
  duration: number;
  start: number;
  end: number;
  lang: string;
}

export default class Playlist {
  private hls: HlsLoader;
  private player: any;
  public streams: Stream[];
  public currentStream: Stream | null;
  private dvrWindow: number;
  private _segmentPointer: number;
  private readonly TAG_NAME: string;

  constructor(hls: HlsLoader) {
    this.hls = hls;
    this.player = hls.player;
    this.streams = [];
    this.currentStream = null;
    this.dvrWindow = 0;
    this._segmentPointer = -1;
    this.TAG_NAME = 'HlsPlaylist';
  }

  public destroy(): void {
    this.reset();
  }

  public get lastSegment(): Segment | null {
    return this.currentStream?.lastSegment || null;
  }

  public get currentSegment(): Segment | undefined {
    return this.currentSegments?.[this._segmentPointer];
  }

  public get nextSegment(): Segment | undefined {
    return this.currentSegments?.[this._segmentPointer + 1];
  }

  public get currentSegments(): Segment[] | undefined {
    return this.currentStream?.segments;
  }

  public get currentSubtitleEndSn(): number {
    return this.currentStream?.currentSubtitleEndSn || 0;
  }

  public get liveEdge(): number {
    return this.currentStream?.liveEdge || 0;
  }

  public get totalDuration(): number {
    return this.currentStream?.totalDuration || 0;
  }

  public get seekRange(): [number, number] | undefined {
    const segments = this.currentSegments;
    if (!segments || !segments.length) return undefined;
    return [
      segments[0].start,
      segments[segments.length - 1].end
    ];
  }

  public get isEmpty(): boolean {
    return !this.currentSegments?.length;
  }

  public get isLive(): boolean {
    return !!this.currentStream?.live;
  }

  public get hasSubtitle(): boolean {
    return !!this.currentStream?.currentSubtitleStream;
  }

  public getAudioSegment(seg: Segment): Segment | undefined {
    return this.currentStream?.getAudioSegment(seg);
  }

  public moveSegmentPointer(pos?: number): void {
    if (pos === null || pos === undefined) pos = this._segmentPointer + 1;
    this._segmentPointer = clamp(pos, -1, this.currentSegments?.length || 0);
    this.player.debug.log(this.TAG_NAME, `moveSegmentPointer() and param pos is ${pos} and clamp result is ${this._segmentPointer}`);
  }

  public reset(): void {
    this.streams = [];
    this.currentStream = null;
    this.dvrWindow = 0;
    this._segmentPointer = -1;
  }

  public getSegmentByIndex(index: number): Segment | undefined {
    return this.currentSegments?.[index];
  }

  public setNextSegmentByIndex(index: number = 0): void {
    this._segmentPointer = index - 1;
    this.player.debug.log(this.TAG_NAME, 'setNextSegmentByIndex()', index, this._segmentPointer);
  }

  public findSegmentIndexByTime(time: number): number | undefined {
    const segments = this.currentSegments;

    if (segments) {
      for (let i = 0, l = segments.length; i < l; i++) {
        const seg = segments[i];
        if (time >= seg.start && time < seg.end) {
          return i;
        }
      }

      const lastSegment = segments[segments.length - 1];
      if (lastSegment && Math.abs(time - lastSegment.end) < 0.2) {
        return segments.length - 1;
      }
    }
    return undefined;
  }

  public upsertPlaylist(
    playlist: Playlist,
    audioPlaylist?: Playlist,
    subtitlePlaylist?: Playlist
  ): void {
    if (!playlist) {
      this.player.debug.warn(this.TAG_NAME, 'upsertPlaylist() playlist is null');
      return;
    }

    if (playlist.isMaster) {
      // streams
      this.streams.length = playlist.streams?.length || 0;

      playlist.streams?.filter(x => x.url).forEach((stream, i) => {
        if (this.streams[i]) {
          this.streams[i].update(stream);
        } else {
          this.streams[i] = new Stream(stream);
        }
      });

      this.currentStream = this.streams[0];
    } else if (Array.isArray(playlist.segments)) {
      // current stream
      const stream = this.currentStream;

      if (stream) {
        stream.update(playlist, audioPlaylist, subtitlePlaylist);

        const newSubtitleSegs = stream.updateSubtitle(subtitlePlaylist);

        if (newSubtitleSegs) {
          this.hls.emit(HLS_EVENTS.SUBTITLE_SEGMENTS, {
            list: newSubtitleSegs
          });
        }
      } else {
        this.reset();
        this.currentStream = this.streams[0] = new Stream(playlist, audioPlaylist, subtitlePlaylist);
      }
    }

    const currentStream = this.currentStream;

    if (currentStream && this.hls.isLive && !this.dvrWindow) {
      this.dvrWindow = this.currentSegments?.reduce((a, c) => {
        return a + c.duration;
      }, 0) || 0;
    }
  }

  public switchSubtitle(lang: string): void {
    this.currentStream?.switchSubtitle(lang);
  }

  public clearOldSegment(maxPlaylistSize: number = 50): void {
    const stream = this.currentStream;

    if (!this.dvrWindow || !stream) return;

    const startTime = stream.endTime - this.dvrWindow;

    if (startTime <= 0) {
      this.player.debug.log(this.TAG_NAME, `clearOldSegment() stream.endTime:${stream.endTime}, this.dvrWindow:${this.dvrWindow}  startTime <= 0`);
      return;
    }

    const segments = stream.segments;

    if (segments.length <= maxPlaylistSize) {
      this.player.debug.log(this.TAG_NAME, `clearOldSegment() segments.length:${segments.length} <= maxPlaylistSize:${maxPlaylistSize}`);
      return;
    }

    const _oldSegmentPointer = this._segmentPointer;
    this._segmentPointer = stream.clearOldSegment(startTime, _oldSegmentPointer);

    this.player.debug.log(this.TAG_NAME, 'clearOldSegment() update _segmentPointer:', _oldSegmentPointer, this._segmentPointer);
    this.player.debug.log(this.TAG_NAME, 'currentSegments', this.currentSegments);
  }

  public checkSegmentTrackChange(cTime: number, nbSb: number): Segment | undefined {
    const index = this.findSegmentIndexByTime(cTime);
    const seg = index !== undefined ? this.getSegmentByIndex(index) : undefined;

    if (!seg) return undefined;
    if (!seg.hasAudio && !seg.hasVideo) return undefined;

    // when seek
    if (nbSb !== 2 && seg.hasAudio && seg.hasVideo) return seg;

    // continuous play
    if (seg.end - cTime > 0.3) return undefined;

    const next = this.getSegmentByIndex(index + 1);

    if (!next) return undefined;
    if (!next.hasAudio && !next.hasVideo) return undefined;
    if (next.hasAudio !== seg.hasAudio || next.hasVideo !== seg.hasVideo) return next;

    return undefined;
  }
} 