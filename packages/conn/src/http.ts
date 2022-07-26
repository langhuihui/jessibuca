import { ChangeState, FSM, Includes } from "afsm";
import { Connection } from ".";
import { ConnectionState } from "./types";

export class HttpFSM extends FSM {
  abortCtrller?: AbortController;
  constructor(public conn: Connection) {
    super(conn.name, "Http");
  }
  @ChangeState([ConnectionState.DISCONNECTED, FSM.INIT], ConnectionState.CONNECTED)
  async connect(url: string) {
    this.abortCtrller = new AbortController();
    const res = await fetch(url, { signal: this.abortCtrller.signal });
    if (!res.body) throw new Error('no body');
    return res.body;
  }

  @ChangeState(ConnectionState.CONNECTED, ConnectionState.DISCONNECTED)
  async disconnect(reason: string) {
    return this.conn.disconnect(reason);
  }
  @ChangeState([], FSM.INIT)
  close() {
    this.abortCtrller?.abort('close');
  }
  @Includes(ConnectionState.CONNECTED)
  send(data: Uint8Array | ArrayBuffer) {

  }
}