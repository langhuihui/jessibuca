import { ERR, StreamingError } from "../../error";
import { concatUint8Array } from "../../utils";
import HlsTsLoader from '../../../demux/hlsTsLoader';
import HlsFmp4Loader from "../../../demux/hlsFmp4Loader";
import { HlsLoader } from '../../index';

interface Player {
  debug: {
    log: (...args: any[]) => void;
  };
}

interface Demuxer {
  destroy: () => void;
  demux: (videoChunk: Uint8Array | null, audioChunk: Uint8Array | null) => void;
  demuxAndFix: (data: Uint8Array, discontinuity: boolean, contiguous: boolean, startTime: number) => void;
  _baseDts?: number;
  getBuferredDuration?: () => number;
  getSampleListLength?: () => number;
  getSampleAudioListLength?: () => number;
  getSampleVideoListLength?: () => number;
}

export default class Transmuxer {
  private readonly hls: HlsLoader;
  private readonly player: Player;
  private readonly isMP4: boolean;
  private _initSegmentId: string;
  private readonly TAG_NAME: string;
  public _demuxer: Demuxer;

  constructor(hls: HlsLoader, isMP4: boolean) {
    this.hls = hls;
    this.player = this.hls.player;
    this.isMP4 = isMP4;

    this._initSegmentId = '';
    this.TAG_NAME = 'HlsTransmuxer';

    if (isMP4) {
      this._demuxer = new HlsFmp4Loader(this.hls.player);
    } else {
      this._demuxer = new HlsTsLoader(this.hls.player);
    }

    this.player.debug.log(this.TAG_NAME, `init and isMP4 is ${isMP4}`);
  }

  public destroy(): void {
    if (this._demuxer) {
      this._demuxer.destroy();
      this._demuxer = null as any;
    }
  }

  public transmux(
    videoChunk: Uint8Array | null,
    audioChunk: Uint8Array | null,
    discontinuity: boolean,
    contiguous: boolean,
    startTime: number,
    needInit: boolean
  ): void {
    this.player.debug.log(
      this.TAG_NAME,
      `transmux videoChunk:${videoChunk && videoChunk.byteLength}, audioChunk:${audioChunk && audioChunk.byteLength
      }, discontinuity:${discontinuity}, contiguous:${contiguous}, startTime:${startTime}, needInit:${needInit}`
    );

    try {
      if (this.isMP4) {
        this._demuxer.demux(videoChunk, audioChunk);
      } else {
        this._demuxer.demuxAndFix(
          concatUint8Array(videoChunk, audioChunk),
          discontinuity,
          contiguous,
          startTime
        );
      }
    } catch (error) {
      throw new StreamingError(ERR.DEMUX, ERR.HLS, error);
    }
  }
} 