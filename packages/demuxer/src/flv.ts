export interface FlvTag {
  type: number;
  data: Uint8Array;
  timestamp: number;
}
export interface FlvReader {
  read<T extends number | Uint8Array>(need: T): Promise<Uint8Array>;
}
export class FlvDemuxer extends ReadableStream<FlvTag> {
  header?: Uint8Array;
  constructor(reader: FlvReader) {
    const t = new Uint8Array(15);//复用15个字节,前面4个字节是上一个tag的长度，跳过
    const tmp8 = new Uint8Array(4);
    const dv = new DataView(tmp8.buffer);
    const readTag = async (controller: ReadableStreamDefaultController<FlvTag>) => {
      await reader.read(t);
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
      return reader.read(length).then(data => {
        controller.enqueue({ type, data: data.slice(), timestamp });
      }).catch(err => controller.error(err));
    };
    const readHead = reader.read(9);
    super({
      start: async controller => {
        console.time('flv');
        const header = await readHead;//跳过flv头
        if (header[0] != 'F'.charCodeAt(0) || header[1] != 'L'.charCodeAt(0) || header[2] != 'V'.charCodeAt(0)) {
          return controller.error(new Error('not flv'));
        }
        console.timeEnd('flv');
        return readTag(controller);
        // conn.once(FSM.INIT, () => controller.close());
        // conn.on(ConnectionState.RECONNECTED, firstRead);
        // return new Promise((resolve, reject) => {
        //   if (conn.state !== ConnectionState.CONNECTED) {
        //     conn.once(ConnectionState.CONNECTED, resolve);
        //   } else {
        //     resolve(null);
        //   }
        // }).then(firstRead);
      },
      pull: readTag
    });
    readHead.then(data => this.header = data);
  }
}