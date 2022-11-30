export { WebSocketConnection } from "./ws";
export { WebTransportConnection } from "./wt";
export { HttpConnection } from "./http";
export { WebRTCConnection, DataChannelConnection, WebRTCStream } from "./webrtc";
export { FileConnection } from './file';
export const enum Protocol {
  WS = "ws://",
  WSS = "wss://",
  HTTP = "http://",
  HTTPS = "https://",
  WebTransport = "wt://",
  WebRTC = "webrtc://",
}
export function getURLType(url: string | File) {
  if (url instanceof File) {
    return "file";
  }
  if (url.endsWith(".flv")) {
    return "flv";
  }
  if (url.startsWith(Protocol.WS) || url.startsWith(Protocol.WSS)) {
    return 'ws';
  } else if (url.startsWith(Protocol.HTTP) || url.startsWith(Protocol.HTTPS)) {
    return 'http';
  } else if (url.startsWith(Protocol.WebTransport)) {
    // url = url.replace(Protocol.WebTransport, Protocol.HTTPS);
    return 'wt';
  } else if (url.startsWith(Protocol.WebRTC)) {
    // url = url.replace(Protocol.WebRTC, this.options.tls ? Protocol.HTTPS : Protocol.HTTP);
    return 'webrtc';
  } else {
    throw new Error("url is invalid");
  }
}
