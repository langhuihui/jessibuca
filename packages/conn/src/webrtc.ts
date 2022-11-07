import { Connection } from "./base";
export class DataChannelConnection extends Connection {
  constructor(public dc: RTCDataChannel) {
    super(dc.label);
  }
  async _connect(): Promise<void | ReadableStream<Uint8Array>> {
    return new ReadableStream({
      start: (controller) => {
        this.dc.onclose = () => {
          controller.close();
        };
        this.dc.onerror = (e) => {
          controller.error(e);
        };
        this.dc.onmessage = (evt) => {
          controller.enqueue(evt.data);
        };
      },
    });
  }
  _close() {
    this.dc.close();
  }
  _send(data: ArrayBufferLike | ArrayBufferView | string | Blob) {
    //@ts-ignore
    this.dc.send(data);
  }
}
export class WebRTCConnection extends Connection {
  webrtc = new RTCPeerConnection(this.options.rtcConfig);
  async _connect() {
    const offer = await this.webrtc.createOffer();
    await this.webrtc.setLocalDescription(offer);
    const res = await fetch(this.url, {
      method: "POST",
      body: offer.sdp,
      ...(this.options.requestInit || {}),
    });
    const answer = await res.text();
    await this.webrtc.setRemoteDescription({ type: "answer", sdp: answer });
    return new Promise<void>((resolve, reject) => {
      this.webrtc.onconnectionstatechange = (evt: Event) => {
        switch (this.webrtc.connectionState) {
          case "disconnected":
            this.disconnect(evt);
            break;
          case "connected":
            resolve();
            break;
          case "failed":
            reject(evt);
        }
      };
    });
  }
  _close() {
    this.webrtc?.close();
  }
}
