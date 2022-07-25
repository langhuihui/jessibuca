import { ChangeState, FSM } from "afsm";
import { WebSocketFSM } from "./ws";
import { range, tap, map, fromEvent, concatMap, takeUntil, timer } from 'streamrx';
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
export interface ConnectionOptions {
  reconnectCount?: number; //重连次数
  reconnectTimeout?: (reconnectionCount: number) => number; //重连超时间隔
}

export const enum ConnectionState {
  CONNECTED = "connected",
  DISCONNECTED = "disconnected",
  RECONNECTED = "reconnected",
}

export type ConnectionEvent = ConnectionState | 'connecting' | 'disconnecting' | 'reconnecting';
export class Connection extends FSM {
  readable?: ReadableStream<Uint8Array>;
  netConn?: WebSocketFSM;
  constructor(public url?: string, public options?: ConnectionOptions) {
    super(url, 'Connection');
    if (!this.options) {
      this.options = {};
    }
    if (!this.options.reconnectTimeout) {
      this.options.reconnectTimeout = getReconnectionTimeout;
    }
  }
  @ChangeState(FSM.INIT, ConnectionState.CONNECTED)
  async connect(url?: string) {
    if (url) {
      this.url = url;
    }
    if (!this.url) {
      throw new Error("url is required");
    }
    if (this.url.startsWith("ws://") || this.url.startsWith("wss://")) {
      this.netConn = new WebSocketFSM(this);
    }
    return this.netConn!.connect();
  }
  @ChangeState(ConnectionState.DISCONNECTED, ConnectionState.RECONNECTED)
  async reconnect() {
    return this.netConn!.connect();
  }
  @ChangeState(ConnectionState.CONNECTED, ConnectionState.DISCONNECTED)
  disconnect(reason: string) {
    if (this.options?.reconnectCount) this.reconnectAfter();
  }
  reconnectAfter(delay: number = 1, count: number = 0) {
    setTimeout(() => {
      this.reconnect().catch(() => {
        //重连失败，再次重连
        if (count < this.options!.reconnectCount!)
          this.reconnectAfter(this.options!.reconnectTimeout!(count), count + 1);
      });
    }, delay);
  }
  @ChangeState([], FSM.INIT)
  async close() {
    this.netConn?.close();
  }
}