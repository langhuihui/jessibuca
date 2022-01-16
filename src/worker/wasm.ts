import { AFSM, AFSM_ACTION } from 'afsm';
export class WasmFSM extends AFSM {
  constructor(wasmURL: string) {
    super({ name: "WasmFSM", quickStart: false });
    this.on(AFSM_ACTION.START, () => {
      importScripts(wasmURL);
    });
  }
}