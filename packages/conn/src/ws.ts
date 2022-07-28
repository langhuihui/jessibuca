import { ChangeState, FSM, Includes } from "afsm";
import { Connection } from ".";
import { ConnectionState } from "./types";

export class WebSocketFSM extends FSM {
  ws?: WebSocket;
  constructor(public conn: Connection) {
    super(conn.name, "WebSocket");
  }
  @ChangeState([ConnectionState.DISCONNECTED, FSM.INIT], ConnectionState.CONNECTED)
  connect(url: string) {
    const ws = new WebSocket(url);
    ws.binaryType = 'arraybuffer';
    return new Promise<ReadableStream<Uint8Array>>((resolve, reject) => {
      ws.onerror = reject;
      ws.onopen = () => {
        this.ws = ws;
        resolve(new ReadableStream({
          start: controller => {
            ws.onclose = e => controller.error(e);
            ws.onmessage = evt => controller.enqueue(evt.data);
          }
        }));
      };
    });
  }
  @ChangeState(ConnectionState.CONNECTED, ConnectionState.DISCONNECTED)
  disconnect(err: any) {
  }
  @ChangeState([], FSM.INIT)
  close() {
    this.ws?.close(1000, "close");
  }
  @Includes(ConnectionState.CONNECTED)
  send(data: ArrayBufferLike | ArrayBufferView) {
    this.ws?.send(data);
  }
}