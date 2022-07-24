import { EventEmitter } from "eventemitter3";
export interface ConnectionOptions {

}
export class Connection extends EventEmitter {
  constructor(
    public readonly url: string,
    public readonly options?: ConnectionOptions
  ) {
    super();
  }
}