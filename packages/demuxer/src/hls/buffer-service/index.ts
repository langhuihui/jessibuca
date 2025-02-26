import Decryptor from './decryptor';
import Transmuxer from "./transmuxer";
import HlsTsLoader from '../../demux/hlsTsLoader';
import MP4Parser from "../../demux/mp4Parser";
import { default as HlsLoader } from '../index';

interface Player {
  debug: {
    log: (...args: any[]) => void;
    error: (...args: any[]) => void;
  };
}

interface SoftVideo {
  endOfStream: () => void;
}

export default class BufferService {
  private readonly hls: HlsLoader;
  private readonly player: Player;
  private readonly _decryptor: Decryptor;
  private _transmuxer: Transmuxer | null;
  private _mse: any | null;
  private _softVideo: SoftVideo | null;
  private _sourceCreated: boolean;
  private _needInitSegment: boolean;
  private _directAppend: boolean;
  private readonly TAG_NAME: string;

  constructor(hls: HlsLoader) {
    this.hls = hls;
    this.player = hls.player;
    this._decryptor = new Decryptor(this.hls, this.player);
    this._transmuxer = null;
    this._mse = null;
    this._softVideo = null;
    this._sourceCreated = false;
    this._needInitSegment = true;
    this._directAppend = false;
    this.TAG_NAME = 'HlsBufferService';
  }

  public async destroy(): Promise<void> {
    this._softVideo = null;
    if (this._transmuxer) {
      this._transmuxer.destroy();
      this._transmuxer = null;
    }
  }

  public get baseDts(): number | undefined {
    return this._transmuxer?._demuxer?._baseDts;
  }

  public get nbSb(): number {
    return 0;
  }

  public async updateDuration(duration: number): Promise<void> {
    this.player.debug.log(this.TAG_NAME, 'updateDuration()', duration);
  }

  public getBuferredDuration(): number | undefined {
    return this._transmuxer?._demuxer?.getBuferredDuration();
  }

  public getBufferedSegments(): number | undefined {
    return this._transmuxer?._demuxer?.getSampleListLength();
  }

  public getBufferedAudioSegments(): number | undefined {
    return this._transmuxer?._demuxer?.getSampleAudioListLength();
  }

  public getBufferedVideoSegments(): number | undefined {
    return this._transmuxer?._demuxer?.getSampleVideoListLength();
  }

  public createSource(
    videoChunk: Uint8Array | null,
    audioChunk: Uint8Array | null,
    videoCodec?: string,
    audioCodec?: string
  ): void {
    if (this._sourceCreated) return;
    const chunk = videoChunk || audioChunk;
    if (!chunk) return;
    if (HlsTsLoader.probe(chunk)) {
      if (!this._transmuxer) {
        this._transmuxer = new Transmuxer(this.hls, false);
      }
    } else if (MP4Parser.probe(chunk)) {
      if (!this._transmuxer) {
        this._transmuxer = new Transmuxer(this.hls, true);
      }
    } else {
      this.player.debug.error(this.TAG_NAME, 'createSource error: chunk is not ts');
    }
  }

  public async appendBuffer(
    segment: any,
    audioSegment: any,
    videoChunk: Uint8Array | null,
    audioChunk: Uint8Array | null,
    discontinuity: boolean,
    contiguous: boolean,
    startTime: number
  ): Promise<boolean> {
    if (!videoChunk?.length && !audioChunk?.length) return false;

    if (!this._transmuxer) return false;

    const needInit = this._needInitSegment || discontinuity;
    this._transmuxer.transmux(
      videoChunk,
      audioChunk,
      discontinuity,
      contiguous,
      startTime,
      this._needInitSegment || discontinuity
    );

    return true;
  }

  public async clearAllBuffer(): Promise<void> {
    this.player.debug.log(this.TAG_NAME, 'clearAllBuffer');
    // if (this._mse) {
    //     return this._mse.clearAllBuffer()
    // }
  }

  public decryptBuffer(
    video: Uint8Array | null,
    audio: Uint8Array | null
  ): Promise<[Uint8Array | null, Uint8Array | null]> {
    return this._decryptor.decrypt(video, audio);
  }

  public async reset(reuseMse: boolean = false): Promise<void> {
    this._transmuxer = null;
    this._needInitSegment = true;
    this._directAppend = false;
  }

  public async endOfStream(): Promise<void> {
    if (this._softVideo) {
      this._softVideo.endOfStream();
    }
  }

  public async setLiveSeekableRange(start: number, end: number): Promise<void> {
    // Implementation left empty intentionally
  }

  /**
   * This makes it possible to change codecs or container type mid-stream.
   * @private
   */
  public seamlessSwitch(): void {
    this._needInitSegment = true;
  }
} 