import { ChangeState, FSM, Includes } from "afsm";
import { Connection } from ".";
import { ConnectionState } from "./types";

export class WebTransportFSM extends FSM {
  wt?: WebTransport;
  writer?: WritableStreamDefaultWriter<any>;
  constructor(public conn: Connection) {
    super(conn.name, "WebTransport");
  }
  @ChangeState([ConnectionState.DISCONNECTED, FSM.INIT], ConnectionState.CONNECTED)
  async connect(url: string) {
    const wt = new WebTransport(url);
    await wt.ready;
    const stream = await wt.createBidirectionalStream();
    this.writer = stream.writable.getWriter();
    return stream.readable;
  }
  @ChangeState(ConnectionState.CONNECTED, ConnectionState.DISCONNECTED)
  disconnect(err: any) {
  }
  @ChangeState([], FSM.INIT)
  close() {
    return this.wt?.close();
  }
  @Includes(ConnectionState.CONNECTED)
  async send(data: ArrayBufferLike | ArrayBufferView) {
    return this.writer?.write(data);
  }
}