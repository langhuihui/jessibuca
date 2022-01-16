import { Subject, subject, Sink, ISink, Observable, nothing, deliver } from "fastrx";

export type PortOb<T = unknown> = Subject<T> & Pick<MessagePort, "postMessage">;
/**
 * 将一个MessagePort转换成Observable
 * @param port 需要封装的端口
 * @param origin 是否发送原始的MessageEvent，默认为false即发送data。
 */
export function fromPort<T>(port: MessagePort, origin = false) {
  const s: PortOb<T> = Object.assign(subject<T>(), {
    postMessage: port.postMessage.bind(port)
  });
  port.onmessage = origin ? evt => {
    if (evt.data) s.next(evt as unknown as T);
    else {
      s.complete();
      port.close();
    }
  } : evt => {
    if (evt.data) s.next(evt.data);
    else {
      s.complete();
      port.close();
    }
  };
  port.onmessageerror = err => {
    s.error(err);
    port.close();
  };
  return s;
}
class TakeUntilComplete<T> extends Sink<T> {
  constructor(sink: ISink<T>, control: Observable<unknown>) {
    super(sink);
    const _takeUntil = new Sink<unknown>(sink);
    _takeUntil.next = nothing;
    control(_takeUntil);
  }
}
export const takeUntilComplete = deliver(TakeUntilComplete, 'takeUntilComplete');