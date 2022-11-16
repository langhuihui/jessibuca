import { BaseDemuxer, DemuxEvent } from "./base";

export interface FlvTag {
  type: number;
  data: Uint8Array;
  timestamp: number;
}
export interface FlvReader {
  read<T extends number | Uint8Array>(need: T): Promise<Uint8Array>;
}
export class FlvDemuxer extends BaseDemuxer {
  header?: Uint8Array;
  async readTag(): Promise<{
    type: number;
    data: Uint8Array;
    timestamp: number;
  }> {
    const t = new Uint8Array(15); //复用15个字节,前面4个字节是上一个tag的长度，跳过
    const tmp8 = new Uint8Array(4);
    const dv = new DataView(tmp8.buffer);
    this.readTag = async () => {
      await this.source.read(t);
      const type = t[4]; //tag类型，8是音频，9是视频，18是script
      tmp8[0] = 0; //首位置空，上一次读取可能会有残留数据
      tmp8.set(t.subarray(5, 8), 1);
      const length = dv.getUint32(0); //大端方式读取长度
      tmp8.set(t.subarray(8, 11), 1);
      let timestamp = dv.getUint32(0); //大端方式读取时间戳
      if (timestamp === 0xffffff) {
        //扩展时间戳
        tmp8[0] = t[11]; //最高位
        timestamp = dv.getUint32(0);
      }
      const data = await this.source.read(length);
      return { type, data: data.slice(), timestamp };
    };
    console.time("flv");
    await this.source.read(9).then((data) => {
      this.header = data;
      console.log(data)
      if (
        data[0] != "F".charCodeAt(0) ||
        data[1] != "L".charCodeAt(0) ||
        data[2] != "V".charCodeAt(0)
      ) {
        throw new Error("not flv");
      }
      console.timeEnd("flv");
    });
    return this.readTag();
  }
  async pull(): Promise<void> {
    const value = await this.readTag();
    if (value) {
      switch (value.type) {
        case 8:
          if (!this.audioEncoderConfig) {
            this.audioEncoderConfig = {
              codec:
                { 10: "aac", 7: "pcma", 8: "pcmu" }[value.data[0] >> 4] ||
                "unknown",
              numberOfChannels: 1,
              sampleRate: 44100,
            };
            //TODO: parse audio config
            if (this.audioEncoderConfig.codec == "aac") {
            }
          }
          if (this.audioEncoderConfig.codec == "aac") {
            if (value.data[1] == 0x00) {
              this.emit(
                DemuxEvent.AUDIO_ENCODER_CONFIG_CHANGED,
                value.data.subarray(2)
              );
              return this.pull();
            }
          }
          return this.gotAudio!({
            type: "key",
            data:
              this.audioEncoderConfig.codec == "aac"
                ? value.data.subarray(2)
                : value.data.subarray(1),
            timestamp: value.timestamp,
            duration: 0,
          });
        case 9:
          if (!this.videoEncoderConfig) {
            this.videoEncoderConfig = {
              codec:
                { 7: "avc", 12: "hevc" }[value.data[0] & 0xf] || "unknown",
              width: 0,
              height: 0,
            };
            //TODO: parse video config
          }
          if (value.data[1] == 0x00) {
            this.emit(
              DemuxEvent.VIDEO_ENCODER_CONFIG_CHANGED,
              value.data.subarray(5)
            );
            return this.pull();
          }
          return this.gotVideo!({
            type: value.data[0] >> 4 == 1 ? "key" : "delta",
            data: value.data.subarray(5),
            timestamp: value.timestamp,
            duration: 0,
          });
        default:
          return this.pull();
      }
    }
  }
}
