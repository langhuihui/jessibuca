import { ChangeState, FSM, Includes } from "afsm";
import { ConnectionOptions, ConnectionState } from "./types";
import Oput from "oput";
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
      this._bps = ((this._buffer * 1000) / (now - this.lastTime)) >> 0;
      this._buffer = 0;
      this.lastTime = now;
    }
    this.total += size;
  }
  get bps() {
    return Date.now() - this.lastTime > 5000 ? 0 : this._bps;
  }
}

export abstract class Connection extends FSM {
  oput?: Oput;
  up = new TransmissionStatistics();
  down = new TransmissionStatistics();
  underlyingSink: UnderlyingSink<Uint8Array> = {
    write: async (chunk) => {
      this.down.add(chunk.length);
      await this.oput?.write(chunk);
    },
  };
  abortCtrl?: AbortController;
  constructor(public url: string, public options: ConnectionOptions = {}) {
    super(url || "conn", "Connection");
    if (!this.options.reconnectTimeout) {
      this.options.reconnectTimeout = getReconnectionTimeout;
    }
  }
  @Includes(ConnectionState.CONNECTED)
  read<T extends number | ArrayBufferView>(need: T) {
    return Promise.reject("not connected");
  }
  @ChangeState(
    [ConnectionState.DISCONNECTED, FSM.INIT],
    ConnectionState.CONNECTED
  )
  async connect() {
    this.abortCtrl = new AbortController();
    console.log(`connected: ${this.url}`);
    this.onConnected(await this._connect());
  }
  // 真正实现连接的函数
  abstract _connect(): Promise<ReadableStream<Uint8Array> | void>;
  _close() {}
  _send(data: ArrayBufferLike | ArrayBufferView) {}
  @ChangeState(ConnectionState.DISCONNECTED, ConnectionState.RECONNECTED)
  async reconnect() {
    console.log(`reconnect: ${this.url}`);
    console.time(this.url);
    this.abortCtrl = new AbortController();
    this.onConnected(await this._connect());
  }
  onConnected(readable: ReadableStream<Uint8Array> | void) {
    console.timeEnd(this.url);
    if (!readable) return;
    this.oput = new Oput();
    // @ts-ignore
    this.read = this.oput.read.bind(this.oput);
    return readable
      .pipeTo(new WritableStream(this.underlyingSink), this.abortCtrl)
      .catch((err) => {
        if (this.abortCtrl!.signal.aborted) return;
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
  close() {
    this.abortCtrl?.abort();
    this._close();
  }
  send(data: ArrayBufferLike | ArrayBufferView) {
    this.up.add(data.byteLength - ("byteOffset" in data ? data.byteOffset : 0));
    this._send(data);
  }
}
