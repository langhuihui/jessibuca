export interface ConnectionOptions {
  reconnectCount?: number; //重连次数
  reconnectTimeout?: (reconnectionCount: number) => number; //重连超时间隔
}

export const enum ConnectionState {
  CONNECTED = "connected",
  DISCONNECTED = "disconnected",
  RECONNECTED = "reconnected",
}


export const enum ConnectionEvent {
  Connecting = "connecting",
  Reconnecting = "reconnecting",
}
