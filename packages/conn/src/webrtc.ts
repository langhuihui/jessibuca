import { ChangeState, FSM, Includes } from "afsm";
import { Connection } from ".";
import { ConnectionState } from "./types";

export class WebRTCFSM extends FSM {
  webrtc?: RTCPeerConnection;
  dc?: RTCDataChannel;
  constructor(public conn: Connection) {
    super(conn.name, "WebRTC");
  }
  @ChangeState([ConnectionState.DISCONNECTED, FSM.INIT], ConnectionState.CONNECTED)
  connect(url: string) {
    const peer = this.webrtc = new RTCPeerConnection(this.conn.options.rtcConfig);
    return new Promise<ReadableStream<Uint8Array> | RTCPeerConnection>((resolve, reject) => {
      peer.onconnectionstatechange = (evt: Event) => {
        switch (peer.connectionState) {
          case "disconnected":
            this.disconnect(evt);
            break;
          case "connected":
            if (this.conn.options.dataChannel) {
              peer.ondatachannel = evt => {
                const dc = this.dc = evt.channel;
                dc.onerror = reject;
                dc.onopen = () => {
                  resolve(new ReadableStream<Uint8Array>({
                    start: controller => {
                      dc.onclose = e => controller.error(e);
                      dc.onmessage = evt => controller.enqueue(evt.data);
                    }
                  }));
                };
              };
            } else {
              resolve(peer);
            }
            break;
          case "failed":
            reject(evt);
        }
      };
    });
  }
  @ChangeState(ConnectionState.CONNECTED, ConnectionState.DISCONNECTED)
  disconnect(err: any) {
  }
  @ChangeState([], FSM.INIT)
  close() {
    this.webrtc?.close();
  }
  @Includes(ConnectionState.CONNECTED)
  send(data: ArrayBufferLike | ArrayBufferView) {
    //@ts-ignore
    this.dc?.send(data);
  }
}