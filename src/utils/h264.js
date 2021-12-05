import SPSParser from './h264-sps-parser.js';

//
export function parseAVCDecoderConfigurationRecord(arrayBuffer) {
    const meta = {}
    const v = new DataView(arrayBuffer.buffer);
    let version = v.getUint8(0);  // configurationVersion
    let avcProfile = v.getUint8(1);  // avcProfileIndication
    let profileCompatibility = v.getUint8(2);  // profile_compatibil
    let avcLevel = v.getUint8(3);  // AVCLevelIndication

    if (version !== 1 || avcProfile === 0) {
        // this._onError(DemuxErrors.FORMAT_ERROR, 'Flv: Invalid AVCDecoderConfigurationRecord');

        return;
    }

    const _naluLengthSize = (v.getUint8(4) & 3) + 1;  // lengthSizeMinusOne

    if (_naluLengthSize !== 3 && _naluLengthSize !== 4) {  // holy shit!!!
        // this._onError(DemuxErrors.FORMAT_ERROR, `Flv: Strange NaluLengthSizeMinusOne: ${_naluLengthSize - 1}`);
        return;
    }
    let spsCount = v.getUint8(5) & 31;  // numOfSequenceParameterSets

    if (spsCount === 0) {
        // this._onError(DemuxErrors.FORMAT_ERROR, 'Flv: Invalid AVCDecoderConfigurationRecord: No SPS');
        return;
    } else if (spsCount > 1) {
        // Log.w(this.TAG, `Flv: Strange AVCDecoderConfigurationRecord: SPS Count = ${spsCount}`);
    }

    let offset = 6;
    for (let i = 0; i < spsCount; i++) {
        let len = v.getUint16(offset, false);  // sequenceParameterSetLength
        offset += 2;

        if (len === 0) {
            continue;
        }

        // Notice: Nalu without startcode header (00 00 00 01)
        let sps = new Uint8Array(arrayBuffer.buffer, offset, len);
        offset += len;
        // flv.js作者选择了自己来解析这个数据结构，也是迫不得已，因为JS环境下没有ffmpeg，解析这个结构主要是为了提取 sps和pps。虽然理论上sps允许有多个，但其实一般就一个。
        // packetTtype 为 1 表示 NALU，NALU= network abstract layer unit，这是H.264的概念，网络抽象层数据单元，其实简单理解就是一帧视频数据。
        // pps的信息没什么用，所以作者只实现了sps的分析器，说明作者下了很大功夫去学习264的标准，其中的Golomb解码还是挺复杂的，能解对不容易，我在PC和手机平台都是用ffmpeg去解析的。
        // SPS里面包括了视频分辨率，帧率，profile level等视频重要信息。
        let config = SPSParser.parseSPS(sps);
        if (i !== 0) {
            // ignore other sps's config
            continue;
        }

        meta.codecWidth = config.codec_size.width;
        meta.codecHeight = config.codec_size.height;
        meta.presentWidth = config.present_size.width;
        meta.presentHeight = config.present_size.height;

        meta.profile = config.profile_string;
        meta.level = config.level_string;
        meta.bitDepth = config.bit_depth;
        meta.chromaFormat = config.chroma_format;
        meta.sarRatio = config.sar_ratio;
        meta.frameRate = config.frame_rate;

        if (config.frame_rate.fixed === false ||
            config.frame_rate.fps_num === 0 ||
            config.frame_rate.fps_den === 0) {
            meta.frameRate = {};
        }

        let fps_den = meta.frameRate.fps_den;
        let fps_num = meta.frameRate.fps_num;

        meta.refSampleDuration = meta.timescale * (fps_den / fps_num);

        let codecArray = sps.subarray(1, 4);

        let codecString = 'avc1.';
        for (let j = 0; j < 3; j++) {
            let h = codecArray[j].toString(16);
            if (h.length < 2) {
                h = '0' + h;
            }
            codecString += h;
        }
        // codec
        meta.codec = codecString;
    }

    let ppsCount = v.getUint8(offset);  // numOfPictureParameterSets
    if (ppsCount === 0) {
        // this._onError(DemuxErrors.FORMAT_ERROR, 'Flv: Invalid AVCDecoderConfigurationRecord: No PPS');
        return;
    } else if (ppsCount > 1) {
        // Log.w(this.TAG, `Flv: Strange AVCDecoderConfigurationRecord: PPS Count = ${ppsCount}`);
    }

    offset++;
    for (let i = 0; i < ppsCount; i++) {
        let len = v.getUint16(offset, false);  // pictureParameterSetLength
        offset += 2;

        if (len === 0) {
            continue;
        }
        let pps = new Uint8Array(arrayBuffer.buffer, offset, len);

        // pps is useless for extracting video information
        offset += len;
    }

    meta.videoType = 'avc';

    // meta.avcc = arrayBuffer;
    return meta;
}
