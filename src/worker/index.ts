import * as handler from './handler';
import { fromPort, PortOb } from '../rx';
const ctx = {};
self.onmessage = function ({ data: { type, payload }, ports }: { data: { type: string; payload: any; }; ports: readonly MessagePort[]; }) {
  //@ts-ignore
  const f: (this: typeof ctx, portOb: PortOb | any, payload?: any) => void = handler[type];
  if (ports[0]) f.call(ctx, fromPort(ports[0]), payload);
  else f.call(ctx, payload);
};