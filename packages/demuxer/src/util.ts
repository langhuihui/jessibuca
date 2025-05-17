export const samplingFrequencyIndexMap = [
  96000,
  88200,
  64000,
  48000,
  44100,
  32000,
  24000,
  22050,
  16000,
  12000,
  11025,
  8000,
  7350,
  -1, // reserved
  -1, // reserved
  -1, // reserved
];

/**
 * Convert AVCC format NAL units to Annex-B format
 * @param avcc Input buffer containing AVCC formatted NAL units
 * @param isKeyframe Whether this is a keyframe (to determine if we need to add parameter sets)
 * @param parameterSets Optional array of Uint8Array, where each is an SPS or PPS NAL unit (without start codes). These will be prepended if `isKeyframe` is true.
 * @returns Buffer containing Annex-B formatted NAL units
 */
export function avccToAnnexb(avcc: Uint8Array, isKeyframe: boolean = false, parameterSets?: Uint8Array[]): Uint8Array {
  const startCode = new Uint8Array([0, 0, 0, 1]);
  let totalLength = 0;

  // Calculate length for parameter sets
  if (isKeyframe && parameterSets && parameterSets.length > 0) {
    for (const pSet of parameterSets) {
      if (pSet instanceof Uint8Array && pSet.length > 0) {
        totalLength += pSet.length + 4; // start code + NALU data
      }
    }
  }

  // Calculate length for NALUs from avcc payload
  // Each NALU in AVCC is prefixed by a 4-byte length. In Annex B, this is replaced by a 4-byte start code.
  // So, the total length contribution from the AVCC payload NALUs remains the same.
  // We need to sum (naluLength + 4 for start code) for each NALU.
  // A simpler way: avcc.length already accounts for (4 byte length + nalu data).
  // So, if we iterate through avcc, sum of (nalu_data_length + 4_for_start_code) is what we need.
  // This is equivalent to avcc.length if we consider the 4-byte length field is replaced by a 4-byte start code.

  let avccNalusPayloadLength = 0;
  let avccOffsetScan = 0;
  while (avccOffsetScan < avcc.length) {
    if (avccOffsetScan + 4 > avcc.length) {
      // Malformed AVCC: not enough bytes for length field
      break;
    }
    const naluLength = (avcc[avccOffsetScan] << 24) | (avcc[avccOffsetScan + 1] << 16) | (avcc[avccOffsetScan + 2] << 8) | avcc[avccOffsetScan + 3];
    if (naluLength < 0 || avccOffsetScan + 4 + naluLength > avcc.length) {
      // Malformed AVCC: invalid NALU length or NALU extends beyond buffer
      break;
    }
    avccNalusPayloadLength += naluLength + 4; // 4 for start code + NALU data
    avccOffsetScan += 4 + naluLength;
  }
  totalLength += avccNalusPayloadLength;


  const annexb = new Uint8Array(totalLength);
  let offset = 0;

  // If this is a keyframe, write parameter sets first
  if (isKeyframe && parameterSets && parameterSets.length > 0) {
    for (const pSet of parameterSets) {
      if (pSet instanceof Uint8Array && pSet.length > 0) {
        if (offset + 4 + pSet.length > annexb.length) {
          // Should not happen if totalLength was calculated correctly
          console.error("Error writing parameter set: buffer overflow");
          return annexb.slice(0, offset); // Return what we have
        }
        annexb.set(startCode, offset);
        annexb.set(pSet, offset + 4);
        offset += pSet.length + 4;
      }
    }
  }

  // Convert NAL units: replace length field with start code
  avccOffsetScan = 0;
  while (avccOffsetScan < avcc.length) {
    if (avccOffsetScan + 4 > avcc.length) {
      // Malformed AVCC
      break;
    }
    const naluLength = (avcc[avccOffsetScan] << 24) | (avcc[avccOffsetScan + 1] << 16) | (avcc[avccOffsetScan + 2] << 8) | avcc[avccOffsetScan + 3];
    if (naluLength < 0 || avccOffsetScan + 4 + naluLength > avcc.length) {
      // Malformed AVCC
      break;
    }

    if (offset + 4 + naluLength > annexb.length) {
      // Should not happen if totalLength was calculated correctly
      console.error("Error writing NALU from AVCC: buffer overflow");
      break; 
    }
    annexb.set(startCode, offset);
    annexb.set(avcc.subarray(avccOffsetScan + 4, avccOffsetScan + 4 + naluLength), offset + 4);
    offset += naluLength + 4;
    avccOffsetScan += 4 + naluLength;
  }

  if (offset !== totalLength) {
    // This can happen if AVCC is malformed and processing stops early.
    // Return a sliced buffer containing only the successfully processed data.
    return annexb.slice(0, offset);
  }

  return annexb;
}

export function adtsToAsc(adts: Uint8Array) {
  const profile = ((adts[2] & 0xc0) >>> 6) + 1;
  const samplingFrequencyIndex = (adts[2] & 0x3c) >>> 2;
  const channelConfiguration = ((adts[2] & 0x01) << 2) | ((adts[3] & 0xc0) >>> 6);
  const audioSpecificConfig = new Uint8Array([
    ((profile & 0x03) << 3) | ((samplingFrequencyIndex & 0x0e) >> 1),
    ((samplingFrequencyIndex & 0x01) << 7) | ((channelConfiguration & 0x0f) << 3),
  ]);
  return {
    profile,
    sampleRate: samplingFrequencyIndexMap[samplingFrequencyIndex],
    channel: channelConfiguration,
    audioSpecificConfig
  };
}

/**
 * Convert Annex-B format NAL units to AVCC format
 * @param annexb Input buffer containing Annex-B formatted NAL units
 * @returns Buffer containing AVCC formatted NAL units
 */
export function annexbToAvcc(annexb: Uint8Array): Uint8Array {
  const nalus: { type: number; data: Uint8Array; }[] = [];
  let offset = 0;

  // Find all NAL units
  while (offset < annexb.length) {
    // Find start code
    while (offset < annexb.length - 3 &&
      !(annexb[offset] === 0 && annexb[offset + 1] === 0 &&
        (annexb[offset + 2] === 1 || (annexb[offset + 2] === 0 && annexb[offset + 3] === 1)))) {
      offset++;
    }

    if (offset >= annexb.length - 3) break;

    const startCodeLength = annexb[offset + 2] === 1 ? 3 : 4;
    const naluStart = offset + startCodeLength;
    offset = naluStart;

    // Find next start code
    while (offset < annexb.length - 3 &&
      !(annexb[offset] === 0 && annexb[offset + 1] === 0 &&
        (annexb[offset + 2] === 1 || (annexb[offset + 2] === 0 && annexb[offset + 3] === 1)))) {
      offset++;
    }

    const naluData = annexb.subarray(naluStart, offset);
    const naluType = naluData[0] & 0x1F; // For H.264
    nalus.push({ type: naluType, data: naluData });
  }

  // Calculate total size needed for AVCC format
  let totalSize = 0;
  for (const nalu of nalus) {
    totalSize += 4 + nalu.data.length; // 4 bytes for length
  }

  const avcc = new Uint8Array(totalSize);
  let dstOffset = 0;

  // Write NAL units in AVCC format
  for (const nalu of nalus) {
    const length = nalu.data.length;
    avcc[dstOffset] = (length >> 24) & 0xFF;
    avcc[dstOffset + 1] = (length >> 16) & 0xFF;
    avcc[dstOffset + 2] = (length >> 8) & 0xFF;
    avcc[dstOffset + 3] = length & 0xFF;
    avcc.set(nalu.data, dstOffset + 4);
    dstOffset += 4 + length;
  }

  return avcc;
}

/**
 * Extract SPS and PPS NAL units from AVCC decoder configuration record.
 * AVCC format is typically found in the 'avcC' box in MP4 files for H.264 video.
 * @param avccData Uint8Array containing the AVCC decoder configuration record.
 * @returns Object containing arrays of SPS and PPS NAL units.
 */
export function extractParameterSetsFromAvcc(avccData: Uint8Array): {
  sps: Uint8Array[];
  pps: Uint8Array[];
} {
  const spsNalus: Uint8Array[] = [];
  const ppsNalus: Uint8Array[] = [];
  let offset = 0;

  // Basic validation
  if (!avccData || avccData.length < 7) { // Minimum size for header + numSPS
    console.error("Invalid AVCC data: too short");
    return { sps: spsNalus, pps: ppsNalus };
  }

  // const configurationVersion = avccData[offset++]; // 1 byte
  // const avcProfileIndication = avccData[offset++]; // 1 byte
  // const profileCompatibility = avccData[offset++]; // 1 byte
  // const avcLevelIndication = avccData[offset++]; // 1 byte
  // const lengthSizeMinusOne = avccData[offset++] & 0x03; // last 2 bits
  offset += 5; // Skip version, profile, compatibility, level, lengthSizeMinusOne

  // Number of SPS
  const numOfSPS = avccData[offset++] & 0x1F; // last 5 bits
  for (let i = 0; i < numOfSPS; i++) {
    if (offset + 2 > avccData.length) {
      console.error("Invalid AVCC data: not enough bytes for SPS length");
      break;
    }
    const spsLength = (avccData[offset] << 8) | avccData[offset + 1];
    offset += 2;
    if (offset + spsLength > avccData.length) {
      console.error("Invalid AVCC data: not enough bytes for SPS NAL unit");
      break;
    }
    spsNalus.push(avccData.subarray(offset, offset + spsLength));
    offset += spsLength;
  }

  // Number of PPS
  if (offset + 1 > avccData.length) {
    // It's possible to have no PPS field if numOfSPS was the last field read and caused an error,
    // or if the data is genuinely truncated before numOfPPS.
    if (numOfSPS > 0 && spsNalus.length === numOfSPS) { // Only warn if SPS were successfully parsed
        // console.warn("AVCC data might be truncated before number of PPS");
    }
    return { sps: spsNalus, pps: ppsNalus };
  }
  const numOfPPS = avccData[offset++];
  for (let i = 0; i < numOfPPS; i++) {
    if (offset + 2 > avccData.length) {
      console.error("Invalid AVCC data: not enough bytes for PPS length");
      break;
    }
    const ppsLength = (avccData[offset] << 8) | avccData[offset + 1];
    offset += 2;
    if (offset + ppsLength > avccData.length) {
      console.error("Invalid AVCC data: not enough bytes for PPS NAL unit");
      break;
    }
    ppsNalus.push(avccData.subarray(offset, offset + ppsLength));
    offset += ppsLength;
  }

  return { sps: spsNalus, pps: ppsNalus };
}

/**
 * Extract VPS, SPS, and PPS NAL units from HVCC decoder configuration record.
 * HVCC format is typically found in the 'hvcC' box in MP4 files for H.265/HEVC video.
 * @param hvccData Uint8Array containing the HVCC decoder configuration record.
 * @returns Object containing arrays of VPS, SPS, and PPS NAL units.
 */
export function extractParameterSetsFromHvcc(hvccData: Uint8Array): {
  vps: Uint8Array[];
  sps: Uint8Array[];
  pps: Uint8Array[];
} {
  const vpsNalus: Uint8Array[] = [];
  const spsNalus: Uint8Array[] = [];
  const ppsNalus: Uint8Array[] = [];
  let offset = 0;

  // Basic validation for HVCC header part (first 22 bytes are fixed)
  if (!hvccData || hvccData.length < 23) { // 22 bytes header + 1 byte for numOfArrays
    console.error("Invalid HVCC data: too short for header");
    return { vps: vpsNalus, sps: spsNalus, pps: ppsNalus };
  }

  // Skip HVCC header (22 bytes)
  offset += 22;

  const numOfArrays = hvccData[offset++];
  if (offset + numOfArrays * 3 > hvccData.length) { // Each array entry is at least 3 bytes
      console.error("Invalid HVCC data: numOfArrays inconsistent with data length");
      return { vps: vpsNalus, sps: spsNalus, pps: ppsNalus };
  }

  for (let i = 0; i < numOfArrays; i++) {
    if (offset + 3 > hvccData.length) { // array_completeness (1bit) + reserved (1bit) + NAL_unit_type (6bits) = 1 byte, numNalus (2 bytes)
        console.error("Invalid HVCC data: not enough bytes for NAL unit array header");
        break;
    }
    const nalUnitType = hvccData[offset] & 0x3F; // 6 bits
    offset++;

    const numNalus = (hvccData[offset] << 8) | hvccData[offset + 1];
    offset += 2;
    let innerLoopBroken = false;

    for (let j = 0; j < numNalus; j++) {
      if (offset + 2 > hvccData.length) {
        console.error("Invalid HVCC data: not enough bytes for NAL unit length");
        innerLoopBroken = true;
        break;
      }
      const nalUnitLength = (hvccData[offset] << 8) | hvccData[offset + 1];
      offset += 2;

      if (offset + nalUnitLength > hvccData.length) {
        console.error("Invalid HVCC data: not enough bytes for NAL unit data");
        innerLoopBroken = true;
        break;
      }
      const nalUnit = hvccData.subarray(offset, offset + nalUnitLength);
      offset += nalUnitLength;

      switch (nalUnitType) {
        case 32: // VPS_NUT
          vpsNalus.push(nalUnit);
          break;
        case 33: // SPS_NUT
          spsNalus.push(nalUnit);
          break;
        case 34: // PPS_NUT
          ppsNalus.push(nalUnit);
          break;
        default:
          break;
      }
    }
    if (innerLoopBroken) {
      break; 
    }
  }

  return { vps: vpsNalus, sps: spsNalus, pps: ppsNalus };
}

/**
 * @deprecated This function is deprecated. Use extractParameterSetsFromAvcc or extractParameterSetsFromHvcc instead.
 * Extract SPS and PPS NAL units from Annex-B formatted data
 * @param data Input buffer containing Annex-B formatted NAL units
 * @param isHevc Whether the data is HEVC/H.265 (true) or AVC/H.264 (false)
 * @returns Object containing arrays of SPS and PPS NAL units
 */
export function extractParameterSets(data: Uint8Array, isHevc: boolean = false): {
  sps: Uint8Array[];
  pps: Uint8Array[];
  vps?: Uint8Array[];
} {
  let offset = 0;
  const spsNalus: Uint8Array[] = [];
  const ppsNalus: Uint8Array[] = [];
  const vpsNalus: Uint8Array[] = [];

  while (offset < data.length - 3) {
    // Find start code
    while (offset < data.length - 3 &&
      !(data[offset] === 0 && data[offset + 1] === 0 &&
        (data[offset + 2] === 1 || (data[offset + 2] === 0 && data[offset + 3] === 1)))) {
      offset++;
    }

    if (offset >= data.length - 3) break;

    const startCodeLength = data[offset + 2] === 1 ? 3 : 4;
    const naluStart = offset + startCodeLength;
    offset = naluStart;

    // Find next start code
    while (offset < data.length - 3 &&
      !(data[offset] === 0 && data[offset + 1] === 0 &&
        (data[offset + 2] === 1 || (data[offset + 2] === 0 && data[offset + 3] === 1)))) {
      offset++;
    }

    const naluData = data.subarray(naluStart, offset);

    if (!isHevc) { // H.264
      const naluType = naluData[0] & 0x1F;
      if (naluType === 7) { // SPS
        spsNalus.push(naluData);
      } else if (naluType === 8) { // PPS
        ppsNalus.push(naluData);
      }
    } else { // H.265
      const naluType = (naluData[0] >> 1) & 0x3F;
      if (naluType === 32) { // VPS
        vpsNalus.push(naluData);
      } else if (naluType === 33) { // SPS
        spsNalus.push(naluData);
      } else if (naluType === 34) { // PPS
        ppsNalus.push(naluData);
      }
    }
  }

  return isHevc ? { sps: spsNalus, pps: ppsNalus, vps: vpsNalus } : { sps: spsNalus, pps: ppsNalus };
}