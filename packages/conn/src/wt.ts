import { ChangeState, FSM, Includes } from "afsm";
import { Connection } from "./base";
import { ConnectionState } from "./types";

export class WebTransportConnection extends Connection {
  wt?: WebTransport;
  writer?: WritableStreamDefaultWriter<any>;
  async _connect() {
    const wt = new WebTransport(this.url);
    await wt.ready;
    const stream = await wt.createBidirectionalStream();
    this.writer = stream.writable.getWriter();
    return stream.readable;
  }
  _close() {
    this.wt?.close();
  }
  _send(data: ArrayBufferLike | ArrayBufferView) {
    return this.writer?.write(data);
  }
}
