import { ChangeState, FSM } from "afsm";
import { ConnectionState, Connection } from ".";

export class WebSocketFSM extends FSM {
  ws?: WebSocket;
  constructor(public conn: Connection) {
    super(conn.name, "WebSocket");
  }
  @ChangeState([ConnectionState.DISCONNECTED, FSM.INIT], ConnectionState.CONNECTED)
  connect() {
    const ws = new WebSocket(this.conn.url!);
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
  send(data: Uint8Array | ArrayBuffer) {
    if (this.state == ConnectionState.CONNECTED) {
      this.ws?.send(data);
    }
  }
}