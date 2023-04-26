import { Connection } from "./base";

export class WebSocketConnection extends Connection {
  ws?: WebSocket;
  async _connect() {
    const ws = new WebSocket(this.url!);
    this.ws = ws;
    ws.binaryType = "arraybuffer";
    return new Promise<ReadableStream>((resolve, reject) => {
      ws.onerror = reject;
      ws.onopen = () => {
        resolve(
          new ReadableStream({
            start: (controller) => {
              ws.onclose = (e) => controller.error(e);
              ws.onmessage = (evt) => controller.enqueue(evt.data);
            },
          })
        );
      };
    });
  }
  _close() {
    this.ws?.close(1000, "close");
  }
  _send(data: ArrayBufferLike | ArrayBufferView) {
    this.ws?.send(data);
  }
}
