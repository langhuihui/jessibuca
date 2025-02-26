import { concatUint8Array } from "../../utils";
import CryptoJS from 'crypto-js';
import { uint8ArrayToWordArray, wordArrayToUint8Array } from "../../../utils/crypto";
import { HlsLoader } from '../../index';

interface Player {
  debug: {
    log: (...args: any[]) => void;
  };
}

interface Segment {
  data: Uint8Array;
  key?: Uint8Array;
  keyIv?: Uint8Array;
  map?: Uint8Array;
}

export interface IExternalDecryptor {
  decrypt(data: Uint8Array, key: BufferSource, iv: BufferSource): Promise<Uint8Array>;
}

export default class Decryptor {
  private readonly hls: HlsLoader;
  private readonly player: Player;
  private readonly subtle: SubtleCrypto | null;
  private externalDecryptor: IExternalDecryptor | null;

  constructor(hls: HlsLoader, player: Player) {
    this.hls = hls;
    this.player = player;
    const crypto = window.crypto || (window as any).msCrypto;
    this.subtle = crypto && (crypto.subtle || (crypto as any).webkitSubtle);
    this.externalDecryptor = null;
  }

  public decrypt(
    video: Segment | null,
    audio: Segment | null
  ): Promise<[Uint8Array | null, Uint8Array | null]> {
    if (!video && !audio) return Promise.resolve([null, null]);

    const ret: Promise<Uint8Array | null>[] = [Promise.resolve(null), Promise.resolve(null)];

    if (video) {
      ret[0] = this._decryptSegment(video);
    }
    if (audio) {
      ret[1] = this._decryptSegment(audio);
    }

    return Promise.all(ret) as Promise<[Uint8Array | null, Uint8Array | null]>;
  }

  private async _decryptSegment(seg: Segment): Promise<Uint8Array> {
    let data = seg.data;
    if (seg.key && seg.keyIv) {
      data = await this._decryptData(seg.data, seg.key, seg.keyIv);
    }
    if (!seg.map) return data;

    return concatUint8Array(seg.map, data);
  }

  private async _decryptData(
    data: Uint8Array,
    key: BufferSource,
    iv: BufferSource
  ): Promise<Uint8Array> {
    if (this.externalDecryptor) {
      return await this.externalDecryptor.decrypt(data, key, iv);
    }

    if (this.subtle) {
      const aesKey = await this.subtle.importKey(
        'raw',
        key,
        { name: 'AES-CBC' },
        false,
        ['encrypt', 'decrypt']
      );
      const decryptData = await this.subtle.decrypt(
        { name: 'AES-CBC', iv },
        aesKey,
        data
      );
      return new Uint8Array(decryptData);
    }

    // Fallback to CryptoJS
    const dataWordArray = uint8ArrayToWordArray(data);
    const keyWordArray = uint8ArrayToWordArray(key as Uint8Array);
    const ivWordArray = uint8ArrayToWordArray(iv as Uint8Array);

    const decrypted = CryptoJS.AES.decrypt(
      { ciphertext: dataWordArray },
      keyWordArray,
      {
        iv: ivWordArray,
        mode: CryptoJS.mode.CBC
      }
    );

    return wordArrayToUint8Array(decrypted);
  }
} 