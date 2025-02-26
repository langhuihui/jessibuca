const REGEXP_TAG = /^#(EXT[^:]*)(?::(.*))?$/;
const REGEXP_ATTR = /([^=]+)=(?:"([^"]*)"|([^",]*))(?:,|$)/g;
const REGEXP_ABSOLUTE_URL = /^(?:[a-zA-Z0-9+\-.]+:)?\/\//;
const REGEXP_URL_PAIR = /^((?:[a-zA-Z0-9+\-.]+:)?\/\/[^/?#]*)?([^?#]*\/)?/;

type CodecType = 'audio' | 'video' | 'text';

interface CodecRegexps {
  [key: string]: RegExp[];
  audio: RegExp[];
  video: RegExp[];
  text: RegExp[];
}

const CODECS_REGEXP: CodecRegexps = {
  audio: [/^mp4a/, /^vorbis$/, /^opus$/, /^flac$/, /^[ae]c-3$/],
  video: [/^avc/, /^hev/, /^hvc/, /^vp0?[89]/, /^av1$/],
  text: [/^vtt$/, /^wvtt/, /^stpp/]
};

function getLines(text: string): string[] {
  return text.split(/[\r\n]/).map((x) => x.trim()).filter(Boolean);
}

function parseTag(text: string): [string, string] | undefined {
  const ret = text.match(REGEXP_TAG);
  if (!ret || !ret[1]) return undefined;
  return [ret[1].replace('EXT-X-', ''), ret[2]];
}

function parseAttr(text: string): { [key: string]: string; } {
  const ret: { [key: string]: string; } = {};
  let match = REGEXP_ATTR.exec(text);
  while (match) {
    ret[match[1]] = match[2] || match[3];
    match = REGEXP_ATTR.exec(text);
  }
  return ret;
}

function getAbsoluteUrl(url: string | undefined, parentUrl: string | undefined): string | undefined {
  if (!parentUrl || !url || REGEXP_ABSOLUTE_URL.test(url)) return url;
  const pairs = REGEXP_URL_PAIR.exec(parentUrl);
  if (!pairs) return url;
  if (url[0] === '/') return pairs[1] + url;
  return pairs[1] + pairs[2] + url;
}

function getCodecs(type: CodecType, codecs: string[] | undefined): string | undefined {
  const re = CODECS_REGEXP[type];
  if (!re || !codecs || !codecs.length) return undefined;
  for (let i = 0; i < re.length; i++) {
    for (let j = 0; j < codecs.length; j++) {
      if (re[i].test(codecs[j])) return codecs[j];
    }
  }
  return undefined;
}

class MasterPlaylist {
  public version: number;
  public streams: MasterStream[];
  public readonly isMaster: boolean;

  constructor() {
    this.version = 0;
    this.streams = [];
    this.isMaster = true;
  }
}

enum MediaType {
  Audio = 'AUDIO',
  Video = 'VIDEO',
  SubTitle = 'SUBTITLE',
  ClosedCaptions = 'CLOSED-CAPTIONS'
}

class MediaStream {
  public id: number = 0;
  public url: string = '';
  public default: boolean = false;
  public autoSelect: boolean = false;
  public forced: boolean = false;
  public group: string = '';
  public name: string = '';
  public lang: string = '';
  public segments: MediaSegment[] = [];
  public endSN: number = 0;
}

class AudioStream extends MediaStream {
  public mediaType: MediaType = MediaType.Audio;
  public channels: number = 0;
}

class VideoStream extends MediaStream {
  public mediaType: MediaType = MediaType.Video;
}

class SubTitleStream extends MediaStream {
  public mediaType: MediaType = MediaType.SubTitle;
}

class ClosedCaptionsStream extends MediaStream {
  public mediaType: MediaType = MediaType.ClosedCaptions;
}

class MasterStream {
  public id: number = 0;
  public bitrate: number = 0;
  public width: number = 0;
  public height: number = 0;
  public name: string = '';
  public url: string = '';
  public audioCodec: string = '';
  public videoCodec: string = '';
  public textCodec: string = '';
  public audioGroup: string = '';
  public audioStreams: AudioStream[] = [];
  public subtitleStreams: SubTitleStream[] = [];
  public closedCaptionsStream: ClosedCaptionsStream[] = [];
}

class MediaPlaylist {
  public version: number = 0;
  public url: string = '';
  public type: string = ''; // upper case
  public startCC: number = 0;
  public endCC: number = 0;
  public startSN: number = 0;
  public endSN: number = 0;
  public totalDuration: number = 0;
  public targetDuration: number = 0;
  public live: boolean = true;
  public segments: MediaSegment[] = [];
}

class MediaSegment {
  public sn: number = 0; // Media Sequence Number
  public cc: number = 0;
  public url: string = '';
  public title: string = '';
  public start: number = 0;
  public duration: number = 0;
  public key: MediaSegmentKey | null = null;
  public byteRange: [number, number] | null = null; // [start, end]
  public isInitSegment: boolean = false;
  public initSegment: MediaSegment | null = null;
  public isLast: boolean = false;
  public hasAudio: boolean = false;
  public hasVideo: boolean = false;

  public get end(): number {
    return this.start + this.duration;
  }

  public setTrackExist(v: boolean, a: boolean): void {
    this.hasVideo = v;
    this.hasAudio = a;
  }

  public setByteRange(data: string, prevSegment?: MediaSegment): void {
    this.byteRange = [0, 0];
    const bytes = data.split('@');
    if (bytes.length === 1 && prevSegment && prevSegment.byteRange) {
      this.byteRange[0] = prevSegment.byteRange[1] || 0;
      if (this.byteRange[0]) this.byteRange[0] += 1;
    } else {
      this.byteRange[0] = parseInt(bytes[1]);
    }
    this.byteRange[1] = this.byteRange[0] + parseInt(bytes[0]) - 1;
  }
}

class MediaSegmentKey {
  public method: string = '';
  public url: string = '';
  public iv: Uint8Array | null = null;
  public keyFormat: string = '';
  public keyFormatVersions: string = '';

  constructor(segKey?: MediaSegmentKey) {
    if (segKey instanceof MediaSegmentKey) {
      this.method = segKey.method;
      this.url = segKey.url;
      this.keyFormat = segKey.keyFormat;
      this.keyFormatVersions = segKey.keyFormatVersions;
      if (segKey.iv) this.iv = new Uint8Array(segKey.iv);
    }
  }

  public clone(sn?: number): MediaSegmentKey {
    const key = new MediaSegmentKey(this);
    if (sn !== undefined) key.setIVFromSN(sn);
    return key;
  }

  public setIVFromSN(sn: number): void {
    if (!this.iv && this.method === 'AES-128' && typeof sn === 'number' && this.url) {
      this.iv = new Uint8Array(16);
      for (let i = 12; i < 16; i++) {
        this.iv[i] = (sn >> (8 * (15 - i))) & 0xff;
      }
    }
  }
}

function parseMediaPlaylist(lines: string[], parentUrl?: string): MediaPlaylist {
  const media = new MediaPlaylist();
  media.url = parentUrl || '';

  let curSegment = new MediaSegment();
  let curInitSegment: MediaSegment | null = null;
  let curKey: MediaSegmentKey | null = null;
  let totalDuration = 0;
  let curSN = 0;
  let curCC = 0;
  let index = 0;
  let line: string;
  let endOfList = false;

  while ((line = lines[index++])) {
    if (endOfList) break;

    if (line[0] !== '#') { // url
      curSegment.sn = curSN;
      curSegment.cc = curCC;
      curSegment.url = getAbsoluteUrl(line, parentUrl) || '';
      if (curKey) curSegment.key = curKey.clone(curSN);
      if (curInitSegment) curSegment.initSegment = curInitSegment;
      media.segments.push(curSegment);
      curSegment = new MediaSegment();
      curSN++;
      continue;
    }

    const tag = parseTag(line);
    if (!tag) continue;

    const [tagName, tagValue] = tag;

    switch (tagName) {
      case 'VERSION':
        media.version = parseInt(tagValue);
        break;
      case 'TARGETDURATION':
        media.targetDuration = parseFloat(tagValue);
        break;
      case 'MEDIA-SEQUENCE':
        curSN = parseInt(tagValue);
        media.startSN = curSN;
        break;
      case 'DISCONTINUITY-SEQUENCE':
        curCC = parseInt(tagValue);
        media.startCC = curCC;
        break;
      case 'DISCONTINUITY':
        curCC++;
        break;
      case 'ENDLIST':
        endOfList = true;
        media.live = false;
        break;
      case 'PLAYLIST-TYPE':
        media.type = tagValue;
        media.live = tagValue !== 'VOD';
        break;
      case 'KEY': {
        const attr = parseAttr(tagValue);
        if (attr.METHOD === 'NONE') {
          curKey = null;
        } else {
          curKey = new MediaSegmentKey();
          curKey.method = attr.METHOD;
          curKey.url = getAbsoluteUrl(attr.URI, parentUrl) || '';
          curKey.keyFormat = attr.KEYFORMAT || '';
          curKey.keyFormatVersions = attr.KEYFORMATVERSIONS || '';
          if (attr.IV) {
            curKey.iv = new Uint8Array(16);
            const iv = attr.IV.slice(2).match(/.{2}/g);
            if (iv) {
              for (let i = 0; i < 16; i++) {
                curKey.iv[i] = parseInt(iv[i], 16);
              }
            }
          }
        }
        break;
      }
      case 'MAP': {
        const attr = parseAttr(tagValue);
        curInitSegment = new MediaSegment();
        curInitSegment.url = getAbsoluteUrl(attr.URI, parentUrl) || '';
        curInitSegment.isInitSegment = true;
        if (attr.BYTERANGE) curInitSegment.setByteRange(attr.BYTERANGE);
        if (curKey) curInitSegment.key = curKey.clone();
        break;
      }
      case 'BYTERANGE':
        curSegment.setByteRange(tagValue, media.segments[media.segments.length - 1]);
        break;
      case 'EXTINF':
        const [duration, title] = tagValue.split(',');
        curSegment.duration = parseFloat(duration);
        curSegment.title = title || '';
        curSegment.start = totalDuration;
        totalDuration += curSegment.duration;
        break;
    }
  }

  if (media.segments.length) {
    media.endSN = media.segments[media.segments.length - 1].sn;
    media.endCC = media.segments[media.segments.length - 1].cc;
    media.totalDuration = totalDuration;
  }

  return media;
}

function parseMasterPlaylist(lines: string[], parentUrl?: string): MasterPlaylist {
  const master = new MasterPlaylist();
  let index = 0;
  let line: string;
  let curStream: MasterStream | null = null;

  const audioStreams = new Map<string, AudioStream[]>();
  const subtitleStreams = new Map<string, SubTitleStream[]>();
  const closedCaptionsStreams = new Map<string, ClosedCaptionsStream[]>();

  while ((line = lines[index++])) {
    const tag = parseTag(line);
    if (!tag) continue;

    const [tagName, tagValue] = tag;

    switch (tagName) {
      case 'VERSION':
        master.version = parseInt(tagValue);
        break;
      case 'STREAM-INF': {
        const attr = parseAttr(tagValue);
        curStream = new MasterStream();
        curStream.bitrate = parseInt(attr.BANDWIDTH);
        curStream.width = parseInt(attr.RESOLUTION?.split('x')[0] || '0');
        curStream.height = parseInt(attr.RESOLUTION?.split('x')[1] || '0');
        curStream.name = attr.NAME || '';
        if (attr.CODECS) {
          const codecs = attr.CODECS.split(',').map(x => x.trim());
          curStream.audioCodec = getCodecs('audio', codecs) || '';
          curStream.videoCodec = getCodecs('video', codecs) || '';
          curStream.textCodec = getCodecs('text', codecs) || '';
        }
        if (attr['AUDIO']) {
          curStream.audioGroup = attr['AUDIO'];
          curStream.audioStreams = Array.from(audioStreams.get(attr['AUDIO'])?.values() || []);
        }
        if (attr['SUBTITLES']) {
          curStream.subtitleStreams = Array.from(subtitleStreams.get(attr['SUBTITLES'])?.values() || []);
        }
        if (attr['CLOSED-CAPTIONS'] && attr['CLOSED-CAPTIONS'] !== 'NONE') {
          curStream.closedCaptionsStream = Array.from(closedCaptionsStreams.get(attr['CLOSED-CAPTIONS'])?.values() || []);
        }
        break;
      }
      case 'MEDIA': {
        const attr = parseAttr(tagValue);
        const groupId = attr['GROUP-ID'];
        if (!groupId) break;

        let stream: MediaStream;
        switch (attr.TYPE) {
          case MediaType.Audio:
            stream = new AudioStream();
            (stream as AudioStream).channels = parseInt(attr.CHANNELS || '2');
            if (!audioStreams.has(groupId)) audioStreams.set(groupId, []);
            audioStreams.get(groupId)?.push(stream as AudioStream);
            break;
          case MediaType.SubTitle:
            stream = new SubTitleStream();
            if (!subtitleStreams.has(groupId)) subtitleStreams.set(groupId, []);
            subtitleStreams.get(groupId)?.push(stream as SubTitleStream);
            break;
          case MediaType.ClosedCaptions:
            stream = new ClosedCaptionsStream();
            if (!closedCaptionsStreams.has(groupId)) closedCaptionsStreams.set(groupId, []);
            closedCaptionsStreams.get(groupId)?.push(stream as ClosedCaptionsStream);
            break;
          default:
            continue;
        }

        stream.id = parseInt(attr['TRACK-ID'] || '0');
        stream.group = groupId;
        stream.name = attr.NAME || '';
        stream.lang = attr.LANGUAGE || '';
        stream.default = attr.DEFAULT === 'YES';
        stream.autoSelect = attr.AUTOSELECT === 'YES';
        stream.forced = attr.FORCED === 'YES';
        if (attr.URI) stream.url = getAbsoluteUrl(attr.URI, parentUrl) || '';
        break;
      }
    }

    if (line[0] !== '#' && curStream) {
      curStream.url = getAbsoluteUrl(line, parentUrl) || '';
      master.streams.push(curStream);
      curStream = null;
    }
  }

  return master;
}

export default class M3U8Parser {
  public static parse(text: string = '', parentUrl?: string): MasterPlaylist | MediaPlaylist {
    const lines = getLines(text);
    if (!lines.length) {
      throw new Error('empty m3u8');
    }
    if (lines[0] !== '#EXTM3U') {
      throw new Error('invalid m3u8');
    }
    return M3U8Parser.isMediaPlaylist(text) ? parseMediaPlaylist(lines, parentUrl) : parseMasterPlaylist(lines, parentUrl);
  }

  public static isMediaPlaylist(text: string): boolean {
    return /^#EXTINF:/.test(text.split('\n').find(line => line.startsWith('#EXTINF:')) || '');
  }
} 