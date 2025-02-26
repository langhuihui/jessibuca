import transform from 'sdp-transform';
const videoEncoderList = ['h264', 'av1'];
export const getUplinkSSRCFromSDP = (sdp: string) => {
  const parsedSDP = parse(sdp);
  const result = {
    audioSsrc: 0,
    audioRtxSsrc: 0,
    bigVideoSsrc: 0,
    bigVideoRtxSsrc: 0,
    smallVideoSsrc: 0,
    smallVideoRtxSsrc: 0,
    auxVideoSsrc: 0,
    auxVideoRtxSsrc: 0
  };
  parsedSDP.media.forEach((item, index) => {
    if (item.ssrcs) {
      if (item.ssrcs[0].id) {
        const ssrc = Number(item.ssrcs[0].id);
        // 视频重传 ssrc
        const rtxSsrc = Number(item.ssrcs.filter(item => item.attribute === 'cname')[1]?.id);
        switch (index) {
          case 0:
            result.audioSsrc = ssrc;
            break;
          case 1:
            result.bigVideoSsrc = ssrc;
            result.bigVideoRtxSsrc = rtxSsrc;
            break;
          case 2:
            result.smallVideoSsrc = ssrc;
            result.smallVideoRtxSsrc = rtxSsrc;
            break;
          case 3:
            result.auxVideoSsrc = ssrc;
            result.auxVideoRtxSsrc = rtxSsrc;
            break;
        }
      }
    }
  });
  return result;
};

function parseSDPMedia(media: {
  type: string;
  port: number;
  protocol: string;
  payloads?: string | undefined;
} & transform.MediaDescription) {
  const result = [];
  for (let i = 0; i < media.rtp.length; i++) {
    if (['rtx', 'red', 'ulpfec'].includes(media.rtp[i].codec)) {
      continue;
    }
    const fmtp = media.fmtp.filter(item => item.payload === media.rtp[i].payload)[0];
    result.push({
      payload: media.rtp[i].payload,
      codec: media.rtp[i].codec,
      fmtp: fmtp ? fmtp.config : '',
      rate: media.rtp[i].rate,
      rtx: media.rtp[i + 1]?.codec === 'rtx' ? media.rtp[i + 1].payload : 0,
      rtcpFb: (media?.rtcpFb || [])
        .filter(item => item.payload === media.rtp[i].payload)
        .map(({ type, subtype }) => ({ id: type, params: subtype ? [subtype] : [] }))
    });
  }
  return result;
}

async function getDecodeAbility() {
  const peerConnection = new RTCPeerConnection();
  peerConnection.addTransceiver('video', {
    direction: 'recvonly'
  });
  const offer = await peerConnection.createOffer();
  if (!offer.sdp) return [];
  const parsedSDP = parse(offer.sdp);
  const decoders: ClientAbilityVideoCodec[] = parseSDPMedia(parsedSDP.media[0]);
  peerConnection.close();
  return decoders;
}

/**
 * 解析 offer sdp，发给后台。
 */
export const getClientAbility = async (sdp: string) => {
  const parsedSDP = parse(sdp);
  const data: ClientAbility = {
    ice: {
      ufrag: '',
      password: ''
    },
    dtls: {
      hash: '',
      fingerprint: '',
      setup: '' // active actpass passive, sdk 默认服务端
    },
    audio: {
      codecs: [],
      extensions: []
    },
    video: {
      codecs: [],
      decoders: [],
      extensions: []
    },
    useDataChannel: false
  };
  // 数字型的 iceUfrag 会被 sdp transform 解析成 number 类型，后台期望是收到 string 类型，否则会进房失败。
  // 此处强制转为 string 类型
  data.ice.ufrag = String(parsedSDP.media[0].iceUfrag);
  data.ice.password = parsedSDP.media[0].icePwd || '';
  if (parsedSDP.fingerprint) {
    data.dtls.hash = parsedSDP.fingerprint.type;
    data.dtls.fingerprint = parsedSDP.fingerprint.hash;
    data.dtls.setup = parsedSDP.setup || '';
  }
  if (parsedSDP.media[0].fingerprint) {
    data.dtls.hash = parsedSDP.media[0].fingerprint.type;
    data.dtls.fingerprint = parsedSDP.media[0].fingerprint.hash;
  }
  data.dtls.setup = parsedSDP.media[0].setup || '';
  const audioItem = parsedSDP.media[0];
  const videoItem = parsedSDP.media[1];
  if (audioItem.ext) {
    data.audio.extensions = audioItem.ext.map(item => ({ id: item.value, uri: item.uri }));
  }
  if (videoItem.ext) {
    data.video.extensions = videoItem.ext.map(item => ({ id: item.value, uri: item.uri }));
  }

  const audioCodec: ClientAbilityAudioCodec = {
    codec: audioItem.rtp[0].codec,
    fmtp: audioItem.fmtp[0].config,
    payload: audioItem.fmtp[0].payload,
    rate: audioItem.rtp[0].rate,
    channel: audioItem.rtp[0].encoding,
    rtcpFb: [],
    rtx: 0
  };
  audioItem.rtcpFb?.forEach(({ payload, type, subtype }) => {
    if (payload === audioCodec.payload) {
      const rtcpFbItem: { id: string, params: string[] } = { id: type, params: [] };
      if (subtype) rtcpFbItem.params.push(subtype);
      audioCodec.rtcpFb.push(rtcpFbItem);
    }
  });
  data.audio.codecs.push(audioCodec);
  
  data.video.codecs = [...parseSDPMedia(videoItem)].filter(item => videoEncoderList.includes(item.codec.toLocaleLowerCase()));
  data.video.decoders = (await getDecodeAbility()).filter(item => videoEncoderList.includes(item.codec.toLocaleLowerCase()));
  return data;
};

/**
 * 将后台返回的 answer sdp config 转成 answer sdp
 */
export const getAnswerSDPFromServerAbility = ({ serverAbility, clientAbility, offerSDP, enableCustomMessage }: { serverAbility: ServerAbility, clientAbility: ClientAbility; offerSDP: string, enableCustomMessage: boolean }) => {
  const parsedOffer = parse(offerSDP);
  const data: transform.SessionDescription = {
    // @ts-ignore
    extmapAllowMixed: 'extmap-allow-mixed',
    groups: parsedOffer.groups,
    icelite: 'ice-lite',
    media: [],
    msidSemantic: { semantic: '', token: 'WMS' },
    name: '-',
    origin: {
      address: '127.0.0.1',
      username: '-',
      sessionId: String(Date.now()),
      sessionVersion: 1,
      netType: 'IN',
      ipVer: 4
    },
    timing: { start: 0, stop: 0 },
    version: 0
  };

  const audio = {
    candidates: serverAbility.candidates.map(item => ({
      component: 1,
      foundation: '1',
      generation: 0,
      ip: item.ip,
      port: item.port,
      priority: item.priority,
      transport: item.foundation,
      type: item.type
    })),
    connection: { version: 4, ip: '0.0.0.0' },
    direction: 'recvonly',
    ext: serverAbility.audio.extensions.map(item => ({ value: item.id, uri: item.uri })),
    fingerprint: { type: serverAbility.dtls.hash, hash: serverAbility.dtls.fingerprint },
    fmtp: [{ payload: serverAbility.audio.codecs[0].payload, config: serverAbility.audio.codecs[0].fmtp }],
    icePwd: serverAbility.ice.password,
    iceUfrag: serverAbility.ice.ufrag,
    mid: '0',
    payloads: String(serverAbility.audio.codecs[0].payload),
    port: parsedOffer.media[0].port,
    protocol: parsedOffer.media[0].protocol,
    type: 'audio',
    setup: serverAbility.dtls.setup,
    rtcpFb: serverAbility.audio.codecs[0].rtcpfb.map(item => ({
      payload: serverAbility.audio.codecs[0].payload,
      type: item.id,
      subtype: item.params[0]
    })),
    rtcpMux: 'rtcp-mux',
    rtcpRsize: 'rtcp-rsize',
    rtp: [
      {
        payload: serverAbility.audio.codecs[0].payload,
        codec: serverAbility.audio.codecs[0].codec,
        rate: serverAbility.audio.codecs[0].rate,
        encoding: serverAbility.audio.codecs[0].channels
      }
    ]
  };

  data.media.push(audio);
  // 固定三路上行视频描述
  [1, 2, 3].forEach(mid => {
    data.media.push(getVideoMediaConfig({ mid, serverAbility, clientAbility, parsedOffer }));
  });
  if (enableCustomMessage) {
    data.media.push(parsedOffer.media.find(item => item.mid === 'dc')!);
  }
  return write(data);
};

/**
 * 获取 video media config，用于生成 answer sdp 中的 video 相关描述。
 * @param {Object} options
 * @param {number} options.mid
 * @param {Object} options.serverAbility 后台给的 ability 对象
 * @param {Object} options.parsedOffer 使用 sdp-transform 库解析出的 offer sdp 对象
 * @param {Object} [options.isDownlink = false] 是否是远端的 sdp
 * 1. 若是获取 answer 中的上行 video media config，则设置为 false。上行的 video codec 根据后台的 useVp8 参数来选择对应的 codec。
 * 2. 若是获取 answer 中的下行 video media config，则设置为 true。下行的 video codec 描述使用后台返回的所有 codec 配置。
 * @returns
 */
export const getVideoMediaConfig = ({ mid, serverAbility, clientAbility, parsedOffer, isDownlink = false }:
  { mid: number, serverAbility: ServerAbility, clientAbility: ClientAbility; parsedOffer: transform.SessionDescription, isDownlink?: boolean }) => {
  const video = {
    candidates: serverAbility.candidates.map(item => ({
      component: 1,
      foundation: '1',
      generation: 0,
      ip: item.ip,
      port: item.port,
      priority: item.priority,
      transport: item.foundation,
      type: item.type
    })),
    connection: { version: 4, ip: '0.0.0.0' },
    direction: 'recvonly',
    ext: serverAbility.video.extensions.map(item => ({ value: item.id, uri: item.uri })),
    fingerprint: { type: serverAbility.dtls.hash, hash: serverAbility.dtls.fingerprint },
    fmtp: [],
    icePwd: serverAbility.ice.password,
    iceUfrag: serverAbility.ice.ufrag,
    mid: String(mid),
    payloads: '',
    port: parsedOffer.media[0].port,
    protocol: parsedOffer.media[0].protocol,
    type: 'video',
    setup: serverAbility.dtls.setup,
    rtcpFb: [],
    rtcpMux: 'rtcp-mux',
    rtcpRsize: 'rtcp-rsize',
    rtp: []
  };

  if (isDownlink) {
    // 下行 offer & answer sdp 中都使用后台返回的 decoders 信息，sdp 中可能会同时包含 264 和 vp8，浏览器会根据实际收到的数据 payload type 来选择解码器。
    // serverAbility.video.codecs 用于兜底，防止后台没返回 decoders
    (serverAbility.video.decoders || serverAbility.video.codecs).forEach(decoder => {
      initCodecConfig(video, decoder);
    });
  } else { // uplink
    const index = serverAbility.video.codecs.findIndex(item => item.codec.toLowerCase() === (serverAbility.useVp8 ? 'vp8' : 'h264'));
    const encoder = serverAbility.video.codecs[index] || clientAbility.video.codecs[0];
    initCodecConfig(video, encoder);
  }

  return video;
};

export const initCodecConfig = (videoMediaConfig: TransformMediaSession, codecConfig: ServerAbilityVideoCodec | ClientAbilityVideoCodec) => {
  videoMediaConfig.payloads = `${videoMediaConfig.payloads} ${codecConfig.payload}`.trim();
  videoMediaConfig.fmtp.push({
    payload: codecConfig.payload,
    config: codecConfig.fmtp
  });
  videoMediaConfig.rtcpFb = [
    ...(videoMediaConfig.rtcpFb || []),
    ...((codecConfig as ServerAbilityVideoCodec).rtcpfb || (codecConfig as ClientAbilityVideoCodec).rtcpFb).map(item => ({
      payload: codecConfig.payload,
      type: item.id,
      subtype: item.params[0]
    }))
  ];
  videoMediaConfig.rtp.push({
    payload: codecConfig.payload,
    codec: codecConfig.codec.toUpperCase(),
    rate: codecConfig.rate
  });
  if (codecConfig.rtx) {
    videoMediaConfig.payloads = `${videoMediaConfig.payloads} ${codecConfig.rtx}`;
    videoMediaConfig.fmtp.push({
      payload: codecConfig.rtx,
      config: `apt=${codecConfig.payload}`
    });
    videoMediaConfig.rtp.push({
      payload: codecConfig.rtx,
      codec: 'rtx',
      rate: codecConfig.rate
    });
  }
};

/**
 * 删除如下 ext line
 * a=extmap:4 urn:ietf:params:rtp-hdrext:sdes:mid
 * a=extmap:5 urn:ietf:params:rtp-hdrext:sdes:rtp-stream-id
 * a=extmap:6 urn:ietf:params:rtp-hdrext:sdes:repaired-rtp-stream-id
 * 原因：不同浏览器对 extmap 解释不同，导致解包失败，进而导致黑屏。
 */
function removeSDESDescription(mediaItem: TransformMediaSession) {
  const extItemsToRemove = new Set([
    'urn:ietf:params:rtp-hdrext:sdes:mid',
    'urn:ietf:params:rtp-hdrext:sdes:rtp-stream-id',
    'urn:ietf:params:rtp-hdrext:sdes:repaired-rtp-stream-id'
  ]);
  if (mediaItem.ext) {
    mediaItem.ext = mediaItem.ext.filter(extItem => !extItemsToRemove.has(extItem.uri));
  }
}

/**
 * 给 sdp 增加 rrtr line
 * 规则：在 a=rtcp-fb:xxx 的最后一行追加 a=rtcp-fb:xxx rrtr
 * 作用：提升弱网抵抗能力
 */
function addRRTRLine(mediaItem: TransformMediaSession) {
  if (!mediaItem.rtcpFb) return;
  const rtcpFb: {
    payload: number;
    type: string;
    subtype?: string | undefined;
  }[] = [];
  mediaItem.rtcpFb.forEach((rtcpFbItem, index) => {
    rtcpFb.push(rtcpFbItem);
    if (mediaItem.rtcpFb && mediaItem.rtcpFb[index + 1]?.payload !== rtcpFbItem.payload && rtcpFbItem.type !== 'rrtr') {
      rtcpFb.push({ payload: rtcpFbItem.payload, type: 'rrtr' });
    }
  });
  mediaItem.rtcpFb = rtcpFb;
}

/**
 * 修复 Chrome h264 开启硬件加速，弱网丢包时拉流出现花屏的问题
 * ref: http://tapd.oa.com/TerWebG/prong/stories/view/1020396022860831661
 */
function addSPSDescription(mediaItem: TransformMediaSession) {
  if (mediaItem.type === 'video' && mediaItem.fmtp) {
    mediaItem.fmtp.forEach(fmtpItem => {
      if (!fmtpItem.config.includes('apt')) {
        fmtpItem.config += ';sps-pps-idr-in-keyframe=1';
      }
    });
  }
}
/**
 * 给 sdp 添加立体声描述
 * sprop-stereo=1 上行立体声
 * stereo=1 下行立体声
 */
function addStereoDescription(mediaItem: TransformMediaSession) {
  if (mediaItem.type === 'audio' && mediaItem.fmtp) {
    mediaItem.fmtp.forEach(fmtpItem => {
      fmtpItem.config += ';sprop-stereo=1;stereo=1';
    });
  }
}

export const handleOfferSDP = (sdp: string, clientAbility: ClientAbility, serverAbility?: ServerAbility) => {
  const parsedSDP = transform.parse(sdp);
  parsedSDP.media.forEach((mediaItem, index) => {
    if (mediaItem.type === 'audio' || mediaItem.type === 'video') {
      addRRTRLine(mediaItem);
      addSPSDescription(mediaItem);
      addStereoDescription(mediaItem);
      removeSDESDescription(mediaItem);
      // 设置 offer sdp 的 codec 信息。index 0 ～ 3 是上行的，其余都是下行的。
      if (mediaItem.type === 'video') {
        if (index < 4) {
          mediaItem.payloads = '';
          mediaItem.fmtp = [];
          mediaItem.rtp = [];
          mediaItem.rtcpFb = [];
          clientAbility.video.codecs.forEach(codec => initCodecConfig(mediaItem, codec));
        } else if (serverAbility) {
          mediaItem.payloads = '';
          mediaItem.fmtp = [];
          mediaItem.rtp = [];
          mediaItem.rtcpFb = [];
          (serverAbility.video.decoders || serverAbility.video.codecs).forEach(codec => initCodecConfig(mediaItem, codec));
        }
      }
    }
    if (mediaItem.payloads?.includes('datachannel') && parsedSDP.groups && mediaItem.mid) {
      parsedSDP.groups[0].mids = parsedSDP.groups[0].mids.replace(mediaItem.mid, 'dc');
      mediaItem.mid = 'dc';
    }
  });
  return transform.write(parsedSDP);
};


interface ClientAbilityAudioCodec {
  codec: string;
  fmtp: string;
  payload: number;
  rate: number | undefined;
  channel: number | undefined;
  rtcpFb: { id: string; params: string[]; }[];
  rtx: number;
}

export interface ClientAbilityVideoCodec { payload: number; codec: string; fmtp: string; rate: number | undefined; rtx: number; rtcpFb: { id: string; params: string[]; }[]; }

export interface ClientAbility {
  ice: {
    ufrag: string,
    password: string
  },
  dtls: {
    hash: string,
    fingerprint: string,
    setup: string // active actpass passive, sdk 默认服务端
  },
  audio: {
    codecs: ClientAbilityAudioCodec[],
    extensions: { id: number; uri: string; }[]
  },
  video: {
    codecs: ClientAbilityVideoCodec[],
    decoders: ClientAbilityVideoCodec[],
    extensions: { id: number; uri: string; }[]
  },
  useDataChannel: boolean
}

interface ServerAbilityAudioCodec {
  codec: string;
  fmtp: string;
  payload: number;
  rate?: number;
  channels?: number;
  rtx: number;
  rtcpfb: { id: string; params: string[]; }[];
}

export interface ServerAbilityVideoCodec { payload: number; codec: string; fmtp: string; rate: number | undefined; rtx: number; rtcpfb: { id: string; params: string[]; }[]; }


export interface ServerAbility {
  audio: {
    codecs: ServerAbilityAudioCodec[];
    extensions: { id: number; uri: string; }[]
  }
  video: {
    codecs: ServerAbilityVideoCodec[],
    decoders: ServerAbilityVideoCodec[],
    extensions: { id: number; uri: string; }[]
  },
  candidates: { ip: string; foundation: string; port: number; priority: number; protocol: string; type: string }[]
  useVp8: boolean; // 开启后，当上行不支持 h264 支持 vp8 时，会使用 vp8 编码。
  forceAllVp8: boolean; // 开启后，当上行支持 vp8 时，会使用 vp8 编解码。
  setupServerRole: boolean;
  useDataChannel: boolean
  ice: {
    ufrag: string,
    password: string
  },
  dtls: {
    hash: string,
    fingerprint: string,
    setup: string // active actpass passive, sdk 默认服务端
  },
}

type TransformMediaSession = {
  type: string;
  port: number;
  protocol: string;
  payloads?: string | undefined;
} & transform.MediaDescription



export const parse = function (sdp: string) {
  return transform.parse(sdp);
};

export const write = function (parsed: ReturnType<typeof transform.parse>) {
  return transform.write(parsed);
};

/**
 * 给 sdp 添加立体声描述
 * sprop-stereo=1 上行立体声
 * stereo=1 下行立体声
 * @param {String} sdp
 * @return {String} sdp
 */
export const addStereoDescription = function (sdp: string) {
  const parsedSDP = parse(sdp);
  parsedSDP.media.forEach(item => {
    if (item.type === 'audio') {
      item.fmtp.forEach(fmtpItem => {
        fmtpItem.config += ';sprop-stereo=1;stereo=1';
      });
    }
  });
  return write(parsedSDP);
};

/**
 * 移除 sdp 中的 h264 描述
 * @export
 * @param {String} sdp
 * @return {String} sdp
 */
export function removeH264(sdp: string) {
  const parsedSDP = parse(sdp);
  parsedSDP.media.forEach(item => {
    if (item.type === 'video') {
      // 需要移除的 payloads
      const payloads = new Set();
      // h264 的 payload 需要移除
      item.rtp.forEach(({ payload, codec }) => codec === 'H264' && payloads.add(payload));
      // apt 为 h264 payload 的也需要移除
      item.fmtp.forEach(({ payload, config }) => {
        const matchApt = config.match(/apt=(\d+)/);
        if (matchApt && matchApt[1] && payloads.has(Number(matchApt[1]))) {
          payloads.add(payload);
        }
      });
      const filterFn = ({ payload } : { payload: number }) => !payloads.has(payload);
      item.rtp = item.rtp.filter(filterFn);
      item.rtcpFb = item.rtcpFb?.filter(filterFn);
      item.fmtp = item.fmtp.filter(filterFn);
      item.payloads = item.payloads
        ?.split(' ')
        .filter(payload => !payloads.has(Number(payload)))
        .join(' ');
    }
  });
  return write(parsedSDP);
}
