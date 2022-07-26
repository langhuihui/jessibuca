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
            ws.onclose = e => {
              controller.close();
              this.disconnect(e.code, e.reason).catch(() => { });
            };
            ws.onmessage = evt => controller.enqueue(evt.data);
          }
        }));
      };
    });
  }
  @ChangeState(ConnectionState.CONNECTED, ConnectionState.DISCONNECTED)
  async disconnect(code = 0, reason: string = '') {
    console.warn('websocket disconnected :', code, reason);
    return this.conn.disconnect(reason);
  }
  @ChangeState([], FSM.INIT)
  close() {
    this.ws?.close(1000, "close");
  }
  @Includes(ConnectionState.CONNECTED)
  send(data: Uint8Array | ArrayBuffer) {
    this.ws?.send(data);
  }
}