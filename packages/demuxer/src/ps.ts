import OPut from "oput";
import { BaseDemuxer, DemuxEvent, DemuxMode } from "./base";
import { adtsToAsc } from './util';
const StartCodePS = 0x000001ba;
const StartCodeSYS = 0x000001bb;
const StartCodeMAP = 0x000001bc;
const StartCodeVideo = 0x000001e0;
const StartCodeAudio = 0x000001c0;
const PrivateStreamCode = 0x000001bd;
const MEPGProgramEndCode = 0x000001b9;
const STREAM_TYPE_H264 = 0x1B;
const STREAM_TYPE_H265 = 0x24;
const STREAM_TYPE_AAC = 0x0F;
const STREAM_TYPE_G711A = 0x90;
const STREAM_TYPE_G711U = 0x91;
export class PSDemuxer extends BaseDemuxer {
  videoStreamType = 0;
  audioStreamType = 0;
  pts = 0;
  dts = 0;
  pull(): Promise<void> {
    throw new Error("Method not implemented.");
  }
  *demux(): Generator<number | Uint8Array, void, Uint8Array> {
    let keySent = false;
    let startTime = 0;
    const tmp8 = new Uint8Array(4);
    const dv = new DataView(tmp8.buffer);
    const videoBuffer = new OPut();
    let currentPTS = 0;
    while (true) {
      yield tmp8;
      const code = dv.getUint32(0);
      switch (code) {
        case StartCodePS:
          yield 9;
          yield tmp8.subarray(0, 1);
          const psl = dv.getUint8(0) & 0x07;
          yield psl;
          break;
        case StartCodeSYS:
        case PrivateStreamCode:
          yield tmp8.subarray(0, 2);
          const sl = dv.getUint16(0);
          yield sl;
          break;
        case StartCodeMAP:
          yield tmp8.subarray(0, 2);
          const msl = dv.getUint16(0);
          const psm = yield msl;
          this.decProgramStreamMap(psm);
          break;
        case StartCodeVideo:
          yield tmp8.subarray(0, 2);
          const vpesl = dv.getUint16(0);
          const vpes = yield vpesl;
          const annexb = this.parsePESPacket(vpes);
          if (!startTime) {
            if ((annexb[4] & 0x0f) == 7) {
              startTime = Date.now();
              currentPTS = this.pts;
              videoBuffer.write(annexb);
            }
            break;
          }
          if (currentPTS == this.pts) {
            videoBuffer.write(annexb);
            break;
          }
          if (videoBuffer.buffer?.length) {
            this.gotVideo?.({
              type: "key",
              data: videoBuffer.buffer!,
              timestamp: (Date.now() - startTime) * 1000,
            });
            videoBuffer.buffer = undefined;
            keySent = true;
          }
          videoBuffer.write(annexb);
          currentPTS = this.pts;
          console.log(annexb[4] & 0x0f);
          break;
        case StartCodeAudio:
          yield tmp8.subarray(0, 2);
          const apesl = dv.getUint16(0);
          const apes = yield apesl;
          const audio = this.parsePESPacket(apes);
          if (this.audioEncoderConfig?.codec == "aac" && this.audioEncoderConfig.numberOfChannels == 0) {
            const asc = adtsToAsc(audio.subarray(7));
            this.audioEncoderConfig = {
              codec: "aac",
              sampleRate: asc.sampleRate,
              numberOfChannels: asc.channel,
            };
            this.emit(
              DemuxEvent.AUDIO_ENCODER_CONFIG_CHANGED,
              asc.audioSpecificConfig
            );
          }
          this.gotAudio?.({
            type: "key",
            data:
              this.audioEncoderConfig?.codec == "aac"
                ? audio.subarray(7)
                : audio,
            timestamp: this.dts,
            duration: 0,
          });
          break;
        case MEPGProgramEndCode:
          return;
      }
    }
  }
  parsePESPacket(payload: Uint8Array) {
    if (payload.length < 4) {
      throw new Error("Short buffer");
    }

    const flag = payload[1];
    const ptsFlag = (flag >> 7) == 1;
    const dtsFlag = ((flag & 0b0100_0000) >> 6) == 1;
    const pesHeaderDataLen = payload[2];

    if (payload.length < pesHeaderDataLen + 3) {
      throw new Error("Short buffer");
    }

    const extraData = payload.subarray(3, 3 + pesHeaderDataLen);

    if (ptsFlag && extraData.length > 4) {
      this.pts =
        ((extraData[0] & 0b0000_1110) << 29) +
        (extraData[1] << 22) +
        ((extraData[2] & 0b1111_1110) << 14) +
        (extraData[3] << 7) +
        (extraData[4] >> 1);

      if (dtsFlag && extraData.length > 9) {
        this.dts =
          ((extraData[5] & 0b0000_1110) << 29) +
          (extraData[6] << 22) +
          ((extraData[7] & 0b1111_1110) << 14) +
          (extraData[8] << 7) +
          (extraData[9] >> 1);
      } else {
        this.dts = this.pts;
      }
    }
    // console.log(this.pts, this.dts);
    return payload.subarray(3 + pesHeaderDataLen);
  }
  decProgramStreamMap(psm: Uint8Array) {
    const dv = new DataView(psm.buffer, psm.byteOffset, psm.byteLength);
    const l = psm.length;
    let index = 2;
    const programStreamInfoLen = dv.getUint16(index);
    index += 2;
    index += programStreamInfoLen;
    let programStreamMapLen = dv.getUint16(index);
    index += 2;
    while (programStreamMapLen > 0) {
      if (l <= index + 1) {
        break;
      }
      const streamType = psm[index];
      index++;
      const elementaryStreamID = psm[index];
      index++;
      if (elementaryStreamID >= 0xe0 && elementaryStreamID <= 0xef) {
        this.videoStreamType = streamType;
        this.videoEncoderConfig = {
          codec:
            { [STREAM_TYPE_H264]: "avc", [STREAM_TYPE_H265]: "hevc" }[streamType] || "unknown",
          width: 0,
          height: 0,
        };
        this.emit(DemuxEvent.VIDEO_ENCODER_CONFIG_CHANGED);
      } else if (elementaryStreamID >= 0xc0 && elementaryStreamID <= 0xdf) {
        this.audioStreamType = streamType;
        this.audioEncoderConfig = {
          codec:
            { [STREAM_TYPE_AAC]: "aac", [STREAM_TYPE_G711A]: "pcma", [STREAM_TYPE_G711U]: "pcmu" }[streamType] ||
            "unknown",
          numberOfChannels: 0,
          sampleRate: 0,
        };
        this.emit(DemuxEvent.AUDIO_ENCODER_CONFIG_CHANGED);
      }
      if (l <= index + 1) {
        break;
      }
      const elementaryStreamInfoLength = dv.getUint16(index);
      index += 2;
      index += elementaryStreamInfoLength;
      programStreamMapLen -= 4 + elementaryStreamInfoLength;
    }
  }
}