import { Connection } from "./base";

export class FileConnection extends Connection {
  constructor(public file: File) {
    super(file.name);
  }
  async _connect() {
    return this.file.stream();
  }
}
