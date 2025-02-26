interface Segment {
  sn: number;
  cc: number;
  url: string;
  duration: number;
  start: number;
  end: number;
}

interface AudioStream {
  default?: boolean;
  segments: Segment[];
  endSN?: number;
}

interface SubtitleStream {
  default?: boolean;
  segments: Segment[];
  endSN?: number;
  lang: string;
}

interface ClosedCaptionsStream {
  // Add properties as needed
}

interface Playlist {
  url: string;
  segments?: Segment[];
  live?: boolean;
  startCC?: number;
  endCC?: number;
  startSN?: number;
  endSN?: number;
  totalDuration?: number;
  targetDuration?: number;
  id?: number;
  bitrate?: number;
  width?: number;
  height?: number;
  name?: string;
  audioCodec?: string;
  videoCodec?: string;
  textCodec?: string;
  audioStreams?: AudioStream[];
  subtitleStreams?: SubtitleStream[];
}

interface SubtitleSegment {
  sn: number;
  url: string;
  duration: number;
  start: number;
  end: number;
  lang: string;
}

export default class Stream {
  public live: boolean | undefined;
  public id: number;
  public bitrate: number;
  public width: number;
  public height: number;
  public name: string;
  public url: string;
  public audioCodec: string;
  public videoCodec: string;
  public textCodec: string;
  public startCC: number;
  public endCC: number;
  public startSN: number;
  public endSN: number;
  public totalDuration: number;
  public targetDuration: number;
  public snDiff: number | null;
  public segments: Segment[];
  public audioStreams: AudioStream[];
  public subtitleStreams: SubtitleStream[];
  public closedCaptions: ClosedCaptionsStream[];
  public currentAudioStream: AudioStream | null;
  public currentSubtitleStream: SubtitleStream | null;
  private readonly TAG_NAME: string;

  constructor(playlist: Playlist, audioPlaylist?: Playlist, subtitlePlaylist?: Playlist) {
    this.live = undefined;
    this.id = 0;
    this.bitrate = 0;
    this.width = 0;
    this.height = 0;
    this.name = '';
    this.url = '';
    this.audioCodec = '';
    this.videoCodec = '';
    this.textCodec = '';

    this.startCC = 0;
    this.endCC = 0;
    this.startSN = 0;
    this.endSN = -1;
    this.totalDuration = 0;
    this.targetDuration = 0;

    this.snDiff = null;
    this.segments = [];
    this.audioStreams = [];
    this.subtitleStreams = [];
    this.closedCaptions = [];
    this.currentAudioStream = null;
    this.currentSubtitleStream = null;
    this.TAG_NAME = 'HlsStream';

    this.update(playlist, audioPlaylist);
  }

  public get lastSegment(): Segment | null {
    if (this.segments.length) {
      return this.segments[this.segments.length - 1];
    }
    return null;
  }

  public get segmentDuration(): number {
    return this.targetDuration || this.segments[0]?.duration || 0;
  }

  public get liveEdge(): number {
    return this.endTime;
  }

  public get endTime(): number {
    return this.lastSegment?.end || 0;
  }

  public get currentSubtitleEndSn(): number {
    return this.currentSubtitleStream?.endSN || 0;
  }

  public clearOldSegment(startTime: number, pointer: number): number {
    return this._clearSegments(startTime, pointer);
  }

  public getAudioSegment(seg?: Segment): Segment | undefined {
    if (!seg || !this.currentAudioStream) return undefined;
    const sn = seg.sn - (this.snDiff || 0);
    return this.currentAudioStream.segments.find(x => x.sn === sn);
  }

  public update(playlist: Playlist, audioPlaylist?: Playlist): void {
    this.url = playlist.url;
    if (Array.isArray(playlist.segments)) { // media
      if (this.live === null || this.live === undefined) {
        this.live = playlist.live;
      }
      this._updateSegments(playlist, this);

      this.startCC = playlist.startCC || 0;
      this.endCC = playlist.endCC || 0;
      this.startSN = playlist.startSN || 0;
      this.endSN = playlist.endSN || -1;
      this.totalDuration = playlist.totalDuration || 0;
      this.targetDuration = playlist.targetDuration || 0;
      this.live = playlist.live;

      if (audioPlaylist && this.currentAudioStream && Array.isArray(audioPlaylist.segments)) {
        this._updateSegments(audioPlaylist, this.currentAudioStream);
        if ((this.snDiff === null || this.snDiff === undefined) && playlist.segments.length && audioPlaylist.segments.length) {
          this.snDiff = playlist.segments[0].sn - audioPlaylist.segments[0].sn;
        }
      }
    } else { // master stream
      this.id = playlist.id || 0;
      this.bitrate = playlist.bitrate || 0;
      this.width = playlist.width || 0;
      this.height = playlist.height || 0;
      this.name = playlist.name || '';
      this.audioCodec = playlist.audioCodec || '';
      this.videoCodec = playlist.videoCodec || '';
      this.textCodec = playlist.textCodec || '';
      this.audioStreams = playlist.audioStreams || [];
      this.subtitleStreams = playlist.subtitleStreams || [];

      if (!this.currentAudioStream && this.audioStreams.length) {
        this.currentAudioStream = this.audioStreams.find(x => x.default) || this.audioStreams[0];
      }

      if (!this.currentSubtitleStream && this.subtitleStreams.length) {
        this.currentSubtitleStream = this.subtitleStreams.find(x => x.default) || this.subtitleStreams[0];
      }
    }
  }

  public updateSubtitle(subtitlePlaylist?: Playlist): SubtitleSegment[] | undefined {
    if (!(subtitlePlaylist && this.currentSubtitleStream && Array.isArray(subtitlePlaylist.segments))) return;

    const newSegs = this._updateSegments(subtitlePlaylist, this.currentSubtitleStream);
    const segs = this.currentSubtitleStream.segments;
    if (segs.length > 100) {
      this.currentSubtitleStream.segments = segs.slice(100);
    }

    if (!newSegs) return;

    return newSegs.map(x => ({
      sn: x.sn,
      url: x.url,
      duration: x.duration,
      start: x.start,
      end: x.end,
      lang: this.currentSubtitleStream!.lang
    }));
  }

  public switchSubtitle(lang: string): void {
    const toSwitch = this.subtitleStreams.find(x => x.lang === lang);
    const origin = this.currentSubtitleStream;
    if (toSwitch) {
      this.currentSubtitleStream = toSwitch;
      if (origin) {
        origin.segments = [];
      }
    }
  }

  private _clearSegments(startTime: number, pointer: number): number {
    let sliceStart = 0;
    const segments = this.segments;
    for (let i = 0, l = segments.length; i < l; i++) {
      if (segments[i].end >= startTime) {
        sliceStart = i;
        break;
      }
    }

    if (sliceStart > pointer) {
      sliceStart = pointer;
    }

    if (sliceStart) {
      this.segments = this.segments.slice(sliceStart);
      if (this.currentAudioStream) {
        this.currentAudioStream.segments = this.currentAudioStream.segments.slice(sliceStart);
      }
    }
    return pointer - sliceStart;
  }

  private _updateSegments(playlist: Playlist, segObj: { segments: Segment[]; endSN?: number; }): Segment[] | undefined {
    const segments = segObj.segments;
    if (this.live) {
      const endSeg = segments[segments.length - 1];
      const endSN = endSeg?.sn ?? -1;

      if (endSN < (playlist.endSN || 0) && playlist.segments?.length) {
        const index = playlist.segments.findIndex(x => x.sn === endSN);
        const toAppend = index < 0 ? playlist.segments : playlist.segments.slice(index + 1);

        if (segments.length && toAppend.length) {
          let endTime = endSeg.end;
          toAppend.forEach(seg => {
            seg.start = endTime;
            endTime = seg.end;
          });

          const lastCC = endSeg?.cc ?? -1;
          if (lastCC > toAppend[0].cc) {
            toAppend.forEach(seg => (seg.cc += lastCC));
          }
        }
        segObj.endSN = playlist.endSN;
        segObj.segments = segments.concat(toAppend);
        return toAppend;
      }
    } else if (playlist.segments) {
      segObj.segments = playlist.segments;
    }
  }
} 