import { Connection } from "./base";

export class HttpConnection extends Connection {
  async _connect() {
    const res = await fetch(this.url!, {
      ...this.options.requestInit,
      signal: this.abortCtrl!.signal,
    });
    if (!res.body) throw new Error("no body");
    return res.body;
  }
}
