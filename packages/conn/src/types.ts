export interface ConnectionOptions {
  reconnectCount?: number; //重连次数
  reconnectTimeout?: (reconnectionCount: number) => number; //重连超时间隔
  tls?: boolean; //webrtc交换sdp时采用https
  rtcConfig?: RTCConfiguration;
  requestInit?: RequestInit;
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
