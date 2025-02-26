import { HLS_EVENTS } from '../events';
import EventEmitter from 'eventemitter3';

interface Sei {
  time: number;
  [key: string]: any;
}

export default class SeiService {
  private emitter: EventEmitter;
  private _seiSet: Set<Sei>;

  constructor(emitter: EventEmitter) {
    this.emitter = emitter;
    this._seiSet = new Set();

    emitter.on(HLS_EVENTS.SEI, (sei: Sei | null) => {
      if (sei) this._seiSet.add(sei);
    });
  }

  public throw(currentTime: number | null | undefined): void {
    if (currentTime === null || currentTime === undefined || !this._seiSet.size) return;

    const min = currentTime - 0.2;
    const max = currentTime + 0.2;
    const toThrow: Sei[] = [];

    this._seiSet.forEach(sei => {
      if (sei.time >= min && sei.time <= max) {
        toThrow.push(sei);
      }
    });

    toThrow.forEach((sei) => {
      this._seiSet.delete(sei);
      // 根据当前视频播放时间抛出 sei，触发该事件表示该 sei 将在当前时间点展示。
      this.emitter.emit(HLS_EVENTS.SEI_IN_TIME, sei);
    });
  }

  public reset(): void {
    this._seiSet.clear();
  }
} 