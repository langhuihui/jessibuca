import {H265_NAL_TYPE} from "../constant";
import SPSParser from "./h265-sps-parser";

export function parseHEVCDecoderConfigurationRecord(arrayBuffer) {
    const meta = {}
    meta.videoType = 'hevc'
    let offset = 28 - 5;
    //
    const vpsTag = arrayBuffer[offset];

    if (vpsTag !== H265_NAL_TYPE.vps) {
        return meta;
    }

    offset += 2;
    offset += 1;
    const vpsLength = arrayBuffer[offset + 1] | (arrayBuffer[offset] << 8);
    offset += 2;
    const vpsData = arrayBuffer.slice(offset, (offset + vpsLength));
    console.log(Uint8Array.from(vpsData));
    offset += vpsLength;

    const spsTag = arrayBuffer[offset]
    if (spsTag !== H265_NAL_TYPE.sps) {
        return meta;
    }
    offset += 2;
    offset += 1;
    const spsLength = arrayBuffer[offset + 1] | (arrayBuffer[offset] << 8);

    offset += 2;
    const spsData = arrayBuffer.slice(offset, (offset + spsLength))
    console.log(Uint8Array.from(spsData));

    offset += spsLength;

    const ppsTag = arrayBuffer[offset];

    if (ppsTag !== H265_NAL_TYPE.pps) {
        return meta;
    }
    offset += 2;
    offset += 1;
    const ppsLength = arrayBuffer[offset + 1] | (arrayBuffer[offset] << 8);
    offset += 2;
    const ppsData = arrayBuffer.slice(offset, (offset + ppsLength))
    console.log(Uint8Array.from(ppsData));

    let sps = Uint8Array.from(spsData);
    let config = SPSParser.parseSPS(sps);

    meta.codecWidth = config.codec_size.width;
    meta.codecHeight = config.codec_size.height;
    meta.presentWidth = config.present_size.width;
    meta.presentHeight = config.present_size.height;

    meta.profile = config.profile_string;
    meta.level = config.level_string;
    meta.bitDepth = config.bit_depth;
    meta.chromaFormat = config.chroma_format;
    meta.sarRatio = config.sar_ratio;

    return meta;
}
