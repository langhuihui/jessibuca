import { Connection } from "../../conn/src";
import { ConnectionState } from "../../conn/src/types";
import { FSM } from 'afsm';
export interface FlvTag {
  type: number;
  data: Uint8Array;
  timestamp: number;
}
export class FlvDemuxer extends ReadableStream<FlvTag> {
  constructor(public conn: Connection) {
    const t = new Uint8Array(15);//复用15个字节,前面4个字节是上一个tag的长度，跳过
    const tmp8 = new Uint8Array(4);
    const dv = new DataView(tmp8.buffer);
    const readTag = async (controller: ReadableStreamDefaultController<FlvTag>) => {
      await conn.read(t);
      const type = t[4];//tag类型，8是音频，9是视频，18是script
      tmp8[0] = 0;//首位置空，上一次读取可能会有残留数据
      tmp8.set(t.subarray(5, 8), 1);
      const length = dv.getUint32(0);//大端方式读取长度
      tmp8.set(t.subarray(8, 11), 1);
      let timestamp = dv.getUint32(0);//大端方式读取时间戳
      if (timestamp === 0xFFFFFF) {//扩展时间戳
        tmp8[0] = t[11];//最高位
        timestamp = dv.getUint32(0);
      }
      return conn.read(length).then(data => {
        controller.enqueue({ type, data: data.slice(), timestamp });
      }).catch(err => controller.error(err));
    };
    super({
      start: async controller => {
        const firstRead = async () => {
          console.time('flv');
          await conn.read(9);//跳过flv头
          return readTag(controller);
        };
        conn.once(FSM.INIT, () => controller.close());
        conn.on(ConnectionState.RECONNECTED, firstRead);
        return new Promise((resolve, reject) => {
          if (conn.state !== ConnectionState.CONNECTED) {
            conn.once(ConnectionState.CONNECTED, resolve);
          } else {
            resolve(null);
          }
        }).then(firstRead);
      },
      pull: readTag
    });
  }
}