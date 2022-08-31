import { ChangeState, FSM, Includes } from "afsm";
import { WebSocketFSM } from "./ws";
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
export const enum Protocol {
  WS = "ws://",
  WSS = "wss://",
  HTTP = "http://",
  HTTPS = "https://",
  WebTransport = "wt://",
  WebRTC = "webrtc://"
}
export class Connection extends FSM {
  type: 'flv' | 'hls' | 'mp4' | 'm7s' | 'unknown' = 'unknown';
  oput?: Oput;
  netConn?: WebSocketFSM | WebTransportFSM | HttpFSM | WebRTCFSM;
  up = new TransmissionStatistics();
  down = new TransmissionStatistics();
  underlyingSink: UnderlyingSink<Uint8Array> = {
    write: async (chunk) => {
      if (!this.down.lastTime) {
        console.timeEnd(this.url);
      }
      this.down.add(chunk.length);
      await this.oput?.write(chunk);
    }
  };
  abortCtrl?: AbortController;
  constructor(public url?: string, public options: ConnectionOptions = {}) {
    super(url || 'conn', 'Connection');
    if (!this.options.reconnectTimeout) {
      this.options.reconnectTimeout = getReconnectionTimeout;
    }
  }

  @ChangeState(FSM.INIT, ConnectionState.CONNECTED)
  async connect(url?: string | File) {
    console.log(`connect: `, url);
    if (url) {
      if (typeof url === 'string')
        this.url = url;
      else {
        this.url = url.name;
      }
    }
    if (!this.url) {
      throw new Error("url is required");
    }
    if (this.url.endsWith('.flv')) {
      this.type = 'flv';
    }
    console.log("connect type: ", this.type);
    console.time(this.url);
    if (url instanceof File) {
      this.onConnected(url.stream());
      return;
    }
    if (this.url.startsWith(Protocol.WS) || this.url.startsWith(Protocol.WSS)) {
      if (!(this.netConn instanceof WebSocketFSM))
        this.netConn = new WebSocketFSM(this);
    } else if (this.url.startsWith(Protocol.HTTP) || this.url.startsWith(Protocol.HTTPS)) {
      if (!(this.netConn instanceof HttpFSM))
        this.netConn = new HttpFSM(this);
    } else if (this.url.startsWith(Protocol.WebTransport)) {
      this.url = this.url.replace(Protocol.WebTransport, Protocol.HTTPS);
      if (!(this.netConn instanceof WebTransportFSM))
        this.netConn = new WebTransportFSM(this);
    } else if (this.url.startsWith(Protocol.WebRTC)) {
      this.url = this.url.replace(Protocol.WebRTC, this.options.tls ? Protocol.HTTPS : Protocol.HTTP);
      if (!(this.netConn instanceof WebRTCFSM))
        this.netConn = new WebRTCFSM(this);
    } else {
      throw new Error("url is invalid");
    }
    return this._connect();
  }
  @ChangeState(ConnectionState.DISCONNECTED, ConnectionState.RECONNECTED)
  async reconnect() {
    console.log(`reconnect: ${this.url}`);
    console.time(this.url);
    return this._connect();
  }
  async _connect() {
    const result = await this.netConn!.connect(this.url!);
    console.log(`connected: ${this.url}`);
    if (result instanceof RTCPeerConnection) {
      return result;
    } else {
      this.onConnected(result);
    }
  }
  onConnected(readable: ReadableStream<Uint8Array>) {
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
    if (this.options.reconnectCount) this.reconnectAfter();
  }
  reconnectAfter(delay: number = 1000, count: number = 0) {
    console.log(`reconnect after ${delay}ms`);
    setTimeout(() => {
      this.reconnect().catch((err) => {
        console.log(`reconnect failed: ${this.url}`, err);
        //重连失败，再次重连
        if (count < this.options.reconnectCount!)
          this.reconnectAfter(this.options.reconnectTimeout!(count), count + 1);
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
  read<T extends number | ArrayBufferView>(need: T) {
    console.log(`read: ${need}`);
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