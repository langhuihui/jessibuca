import ExpGolomb from './exp-golomb.js';

class SPSParser {

    static _ebsp2rbsp(uint8array) {
        let src = uint8array;
        let src_length = src.byteLength;
        let dst = new Uint8Array(src_length);
        let dst_idx = 0;

        for (let i = 0; i < src_length; i++) {
            if (i >= 2) {
                // Unescape: Skip 0x03 after 00 00
                if (src[i] === 0x03 && src[i - 1] === 0x00 && src[i - 2] === 0x00) {
                    continue;
                }
            }
            dst[dst_idx] = src[i];
            dst_idx++;
        }

        return new Uint8Array(dst.buffer, 0, dst_idx);
    }


    static parseSPS(uint8array) {
        let rbsp = SPSParser._ebsp2rbsp(uint8array);
        let gb = new ExpGolomb(rbsp);
        let profile_string = '';
        let level_string = '';
        let bit_depth = '';
        let ref_frames = '';
        let chroma_format = '';
        let fps_fixed = '';
        let fps = '';
        let fps_den = '';
        let fps_num = '';
        let sar_width = '';
        let sar_height = '';
        let codec_width = '';
        let codec_height = '';
        let present_width = '';
        return {
            profile_string: profile_string,  // baseline, high, high10, ...
            level_string: level_string,  // 3, 3.1, 4, 4.1, 5, 5.1, ...
            bit_depth: bit_depth,  // 8bit, 10bit, ...
            ref_frames: ref_frames,
            chroma_format: chroma_format,  // 4:2:0, 4:2:2, ...
            chroma_format_string: '',

            frame_rate: {
                fixed: fps_fixed,
                fps: fps,
                fps_den: fps_den,
                fps_num: fps_num
            },

            sar_ratio: {
                width: sar_width,
                height: sar_height
            },

            codec_size: {
                width: codec_width,
                height: codec_height
            },

            present_size: {
                width: present_width,
                height: codec_height
            }
        };
    }

    static _skipScalingList(gb, count) {
        let last_scale = 8, next_scale = 8;
        let delta_scale = 0;
        for (let i = 0; i < count; i++) {
            if (next_scale !== 0) {
                delta_scale = gb.readSEG();
                next_scale = (last_scale + delta_scale + 256) % 256;
            }
            last_scale = (next_scale === 0) ? last_scale : next_scale;
        }
    }


    static getLevelString(level_idc) {
        return (level_idc / 10).toFixed(1);
    }

}

export default SPSParser
