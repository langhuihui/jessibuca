import { BaseDemuxer, DemuxEvent, DemuxMode } from "./base";
import { samplingFrequencyIndexMap } from "./util";
const FourCC_H265 = "hvc1";
const FourCC_AV1 = "av01";
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
  tmp8 = new Uint8Array(4);
  dv = new DataView(this.tmp8.buffer);
  async pullTag(): Promise<{
    type: number;
    data: Uint8Array;
    timestamp: number;
  }> {
    const t = new Uint8Array(15); //复用15个字节,前面4个字节是上一个tag的长度，跳过
    this.pullTag = async () => {
      await this.source!.read(t);
      const type = t[4]; //tag类型，8是音频，9是视频，18是script
      const length = this.readLength(t.subarray(5, 8));
      const timestamp = this.readTimestamp(t.subarray(8, 11));
      const data = await this.source!.read(length);
      return { type, data: data.slice(), timestamp };
    };
    console.time("flv");
    await this.source!.read(9).then((data) => {
      this.header = data;
      console.log(data);
      if (data.subarray(0, 3).reduce((acc, cur) => acc + String.fromCharCode(cur), "") !== "FLV") {
        throw new Error("not flv");
      }
      console.timeEnd("flv");
    });
    return this.pullTag();
  }
  readTag(data: Uint8Array) {
    const type = data[0]; //tag类型，8是音频，9是视频，18是script
    const length = this.readLength(data.subarray(1, 4));
    const timestamp = this.readTimestamp(data.subarray(4, 8));
    this.gotTag(type, data.subarray(11, 11 + length), timestamp);
  }
  gotTag(type: number, data: Uint8Array, timestamp: number) {
    switch (type) {
      case 8:
        if (!this.audioDecoderConfig) {
          this.audioDecoderConfig = {
            codec:
              { 10: "aac", 7: "pcma", 8: "pcmu" }[data[0] >> 4] || "unknown",
            numberOfChannels: 1,
            sampleRate: 8000,
          };
          if (this.audioDecoderConfig.codec == "aac") {
            this.audioDecoderConfig.numberOfChannels = (data[3] >> 3) & 0x0f; //声道
            this.audioDecoderConfig.sampleRate =
              samplingFrequencyIndexMap[
              ((data[2] & 0x7) << 1) | (data[3] >> 7)
              ];
          } else {
            this.emit(
              DemuxEvent.AUDIO_ENCODER_CONFIG_CHANGED,
              this.audioDecoderConfig
            );
          }
        }
        if (this.audioDecoderConfig.codec == "aac") {
          if (data[1] == 0x00) {
            this.audioDecoderConfig.description = data.subarray(2);
            this.emit(
              DemuxEvent.AUDIO_ENCODER_CONFIG_CHANGED,
              this.audioDecoderConfig
            );
            if (this.mode == DemuxMode.PULL) return this.pull();
            else return;
          }
        }
        return this.gotAudio?.({
          type: "key",
          data:
            this.audioDecoderConfig.codec == "aac"
              ? data.subarray(2)
              : data.subarray(1),
          timestamp: timestamp,
          duration: 0,
        });
      case 9:
        if (data[0] >> 7) {
          // rtmp 扩展协议
          if (data[0] & 0x0f) {
            return this.gotVideo?.({
              type: data[0] >> 4 == 1 ? "key" : "delta",
              data: data.subarray(
                this.videoDecoderConfig.codec == "hevc" ? 8 : 5
              ),
              timestamp: timestamp,
              duration: 0,
            });
          } else {
            switch (
            data
              .subarray(1, 5)
              .reduce((acc, cur) => acc + String.fromCharCode(cur), "")
            ) {
              case FourCC_H265:
                this.videoDecoderConfig = {
                  codec: "hevc",
                  description: data.subarray(5),
                };
                break;
              case FourCC_AV1:
                this.videoDecoderConfig = {
                  codec: "av1",
                };
                break;
            }
            this.emit(
              DemuxEvent.VIDEO_ENCODER_CONFIG_CHANGED,
              this.videoDecoderConfig!
            );
            if (this.mode == DemuxMode.PULL) return this.pull();
            else return;
          }
        } else if (data[1] === 0) {
          this.videoDecoderConfig = {
            codec:
              { 7: "avc", 12: "hevc", 13: "av1" }[data[0] & 0xf] || "unknown",
            description: data.subarray(5),
          };
          if (this.videoDecoderConfig.codec == "av1") {
            delete this.videoDecoderConfig.description;
          }
          this.emit(
            DemuxEvent.VIDEO_ENCODER_CONFIG_CHANGED,
            this.videoDecoderConfig!
          );
          if (this.mode == DemuxMode.PULL) return this.pull();
          else return;
        }
        return this.gotVideo?.({
          type: data[0] >> 4 == 1 ? "key" : "delta",
          data: data.subarray(5),
          timestamp: timestamp,
          duration: 0,
        });
      default:
        if (this.mode == DemuxMode.PULL) return this.pull();
    }
  }
  async pull(): Promise<void> {
    const value = await this.pullTag();
    if (value) {
      return this.gotTag(value.type, value.data, value.timestamp);
    }
  }
  readLength(data: Uint8Array) {
    this.tmp8[0] = 0; //首位置空，上一次读取可能会有残留数据
    this.tmp8.set(data, 1);
    return this.dv.getUint32(0); //大端方式读取长度
  }
  readTimestamp(data: Uint8Array) {
    this.tmp8.set(data.subarray(0, 3), 1);
    let timestamp = this.dv.getUint32(0); //大端方式读取时间戳
    if (timestamp === 0xffffff) {
      //扩展时间戳
      this.tmp8[0] = data[3]; //最高位
      timestamp = this.dv.getUint32(0);
    }
    return timestamp;
  }
  readHead(data: Uint8Array) {
    console.time("flv");
    this.header = data;
    console.log(data);
    if (data.subarray(0, 3).reduce((acc, cur) => acc + String.fromCharCode(cur), "") !== "FLV") {
      throw new Error("not flv");
    }
    console.timeEnd("flv");
  }

  *demux(): Generator<number, void, Uint8Array> {
    this.readHead(yield 13);
    while (true) {
      let data = yield 11;
      const type = data[0]; //tag类型，8是音频，9是视频，18是script
      const length = this.readLength(data.subarray(1, 4));
      const timestamp = this.readTimestamp(data.subarray(4, 8));
      data = yield length;
      this.gotTag(type, data.slice(), timestamp);
      yield 4;
    }
  }
}
