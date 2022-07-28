import { ChangeState, FSM, Includes } from "afsm";
import { WebSocketFSM } from "./ws";
import { range, tap, map, fromEvent, concatMap, takeUntil, timer } from 'streamrx';
import { ConnectionOptions, ConnectionState } from "./types";
import Oput from 'oput';
import { WebTransportFSM } from "./wt";
import { HttpFSM } from "./http";
import { WebRTCFSM } from "./webrtc";
/**
 * 使用尾递归优化，计算斐波那契数
 * @export
 * @param {number} n
 * @param {number} [ac1=1]
 * @param {number} [ac2=1]
 * @return {number}
 */
export function fibonacci(n: number, ac1 = 1, ac2 = 1): number {
  return n <= 1 ? ac2 : fibonacci(n - 1, ac2, ac1 + ac2);
}

/**
 * 获取重连超时间隔
 * 根据斐波那契数来计算，最大超时间隔为 13s，最小间隔为 2s
 * @return {number}
 * @memberof SignalChannel
 */
export function getReconnectionTimeout(reconnectionCount: number) {
  // 最小间隔2s，2,3,5,8 间隔各尝试2次
  const n = Math.round(reconnectionCount / 2) + 1;
  // 最大间隔 13s
  // fibonacci(6) = 13
  return n > 6 ? 13 * 1000 : fibonacci(n) * 1000;
}
export class TransmissionStatistics {
  total: number = 0;
  _buffer: number = 0;
  lastTime: number = 0;
  _bps: number = 0;
  add(size: number) {
    const now = Date.now();
    this._buffer += size;
    if (this.lastTime === 0) {
      this.lastTime = now;
    } else if (now - this.lastTime > 1000) {
      this._bps = this._buffer * 1000 / (now - this.lastTime) >> 0;
      this._buffer = 0;
      this.lastTime = now;
    }
    this.total += size;
  }
  get bps() {
    return Date.now() - this.lastTime > 5000 ? 0 : this._bps;
  }
}
export class Connection extends FSM {
  oput?: Oput;
  netConn?: WebSocketFSM | WebTransportFSM | HttpFSM | WebRTCFSM;
  up = new TransmissionStatistics();
  down = new TransmissionStatistics();
  underlyingSink: UnderlyingSink<Uint8Array> = {
    write: (chunk) => {
      if (!this.down.lastTime) {
        console.timeEnd(this.url);
      }
      this.down.add(chunk.length);
      return this.oput?.write(chunk);
    }
  };
  abortCtrl?: AbortController;
  constructor(public url?: string, public options?: ConnectionOptions) {
    super(url || 'conn', 'Connection');
    if (!this.options) {
      this.options = {};
    }
    if (!this.options.reconnectTimeout) {
      this.options.reconnectTimeout = getReconnectionTimeout;
    }
  }

  @ChangeState(FSM.INIT, ConnectionState.CONNECTED)
  async connect(url?: string | File) {
    if (url) {
      if (typeof url === 'string')
        this.url = url;
      else {
        this.url = url.name;
        console.time(this.url);
        this.onConnected(url.stream());
        return;
      }
    }
    if (!this.url) {
      throw new Error("url is required");
    }
    console.log(`connect: ${this.url}`);
    console.time(this.url);
    if (this.url.startsWith("ws://") || this.url.startsWith("wss://")) {
      if (!(this.netConn instanceof WebSocketFSM))
        this.netConn = new WebSocketFSM(this);
    } else if (this.url.startsWith("http://") || this.url.startsWith("https://")) {
      if (!(this.netConn instanceof HttpFSM))
        this.netConn = new HttpFSM(this);
    } else if (this.url.startsWith("wt://")) {
      this.url = this.url.replace("wt://", "https://");
      if (!(this.netConn instanceof WebTransportFSM))
        this.netConn = new WebTransportFSM(this);
    } else if (this.url.startsWith("webrtc://")) {
      this.url = this.url.replace("webrtc://", "https://");//TODO: http
      if (!(this.netConn instanceof WebRTCFSM))
        this.netConn = new WebRTCFSM(this);
    } else {
      throw new Error("url is invalid");
    }
    this.onConnected(await this.netConn!.connect(this.url));
  }
  @ChangeState(ConnectionState.DISCONNECTED, ConnectionState.RECONNECTED)
  async reconnect() {
    console.log(`reconnect: ${this.url}`);
    console.time(this.url);
    this.onConnected(await this.netConn!.connect(this.url!));
  }
  onConnected(readable: ReadableStream<Uint8Array>) {
    console.log(`connected: ${this.url}`);
    this.oput = new Oput();
    this.abortCtrl = new AbortController();
    readable.pipeTo(new WritableStream(this.underlyingSink), this.abortCtrl).catch((err) => {
      if (this.abortCtrl!.signal.aborted) return;
      this.netConn!.disconnect(err);
      this.disconnect(err);
    });
  }
  @ChangeState(ConnectionState.CONNECTED, ConnectionState.DISCONNECTED)
  disconnect(reason: any) {
    console.warn(`disconnect: ${this.url}`, reason);
    if (this.options?.reconnectCount) this.reconnectAfter();
  }
  reconnectAfter(delay: number = 1000, count: number = 0) {
    console.log(`reconnect after ${delay}ms`);
    setTimeout(() => {
      this.reconnect().catch((err) => {
        console.log(`reconnect failed: ${this.url}`, err);
        //重连失败，再次重连
        if (count < this.options!.reconnectCount!)
          this.reconnectAfter(this.options!.reconnectTimeout!(count), count + 1);
      });
    }, delay);
  }
  @ChangeState([], FSM.INIT)
  async close() {
    this.abortCtrl?.abort();
    this.netConn?.close();
  }
  @Includes(ConnectionState.CONNECTED)
  send(data: ArrayBufferLike | ArrayBufferView) {
    this.up.add(data.byteLength - ('byteOffset' in data ? data.byteOffset : 0));
    this.netConn?.send(data);
  }
  @Includes(ConnectionState.CONNECTED)
  read(need: Parameters<typeof Oput.prototype.read>[0]) {
    return this.oput!.read(need);
  }
  @Includes(ConnectionState.CONNECTED)
  readU32() {
    return this.oput!.readU32();
  }
  @Includes(ConnectionState.CONNECTED)
  readU16() {
    return this.oput!.readU16();
  }
  @Includes(ConnectionState.CONNECTED)
  readU8() {
    return this.oput!.readU8();
  }
}