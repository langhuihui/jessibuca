export interface ConnectionOptions {
  reconnectCount?: number; //重连次数
  reconnectTimeout?: (reconnectionCount: number) => number; //重连超时间隔
  tls?: boolean;//webrtc交换sdp时采用https
  rtcConfig?: RTCConfiguration;
  dataChannel?: boolean;//webrtc使用datachannel传输
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
