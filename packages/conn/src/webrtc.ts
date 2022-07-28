import { ChangeState, FSM, Includes } from "afsm";
import { Connection } from ".";
import { ConnectionState } from "./types";

export class WebRTCFSM extends FSM {
  webrtc?: RTCPeerConnection;
  constructor(public conn: Connection) {
    super(conn.name, "WebRTC");
  }
  @ChangeState([ConnectionState.DISCONNECTED, FSM.INIT], ConnectionState.CONNECTED)
  connect(url: string) {
    return new ReadableStream<Uint8Array>();
  }
  @ChangeState(ConnectionState.CONNECTED, ConnectionState.DISCONNECTED)
  disconnect(err: any) {
  }
  @ChangeState([], FSM.INIT)
  close() {

  }
  @Includes(ConnectionState.CONNECTED)
  send(data: ArrayBufferLike | ArrayBufferView) {
  }
}