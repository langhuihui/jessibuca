// 播放协议
export const PLAYER_PLAY_PROTOCOL = {
    websocket: 0,
    fetch: 1
}

export const DEMUX_TYPE = {
    flv: 'flv',
    m7s: 'm7s'
}


// default player options
export const DEFAULT_PLAYER_OPTIONS = {
    videoBuffer: 1000, //1000ms == 1 second
    isResize: true,
    isFullResize: false, //
    isFlv: false,
    debug: false,
    loadingTimeout: 10, // loading timeout
    heartTimeout: 10,  // heart timeout
    timeout: 10, // second
    supportDblclickFullscreen: false,
    showBandwidth: false, //
    keepScreenOn: false,
    isNotMute: false,
    hasAudio: true,
    hasVideo: true,
    operateBtns: {
        fullscreen: false,
        screenshot: false,
        play: false,
        audio: false,
        record: false
    },
    hasControl: false,
    loadingText: '',
    background: '',
    decoder: 'decoder.js',
    url: '',//
    rotate: 0,
    // text: '',
    forceNoOffscreen: true, // 默认是不采用
    hiddenAutoPause: false,
    protocol: PLAYER_PLAY_PROTOCOL.fetch,
    demuxType: DEMUX_TYPE.flv, //
    useWCS: false, //
    useMSE: false, //
    useOffscreen: false //
}


export const WORKER_CMD_TYPE = {
    init: 'init',
    initVideo: 'initVideo',
    render: 'render',
    playAudio: 'playAudio',
    print: 'print',
    printErr: 'printErr',
    initAudio: 'initAudio',
    kBps: 'kBps',
    decode: 'decode',
    audioCode: 'audioCode',
    videoCode: 'videoCode'
}

export const MEDIA_TYPE = {
    audio: 1,
    video: 2
}

export const FLV_MEDIA_TYPE = {
    audio: 8,
    video: 9
}

export const WORKER_SEND_TYPE = {
    init: 'init',
    decode: 'decode',
    audioDecode: 'audioDecode',
    videoDecode: 'videoDecode',
    close: 'close'
}


//
export const EVENTS = {
    fullscreen: 'fullscreen$2',
    webFullscreen: 'webFullscreen',
    decoderWorkerInit: 'decoderWorkerInit',
    play: 'play',
    playing: 'playing',
    pause: 'pause',
    mute: 'mute',
    load: 'load',
    loading: 'loading',
    videoInfo: 'videoInfo',
    timeUpdate: 'timeUpdate',
    audioInfo: "audioInfo",
    log: 'log',
    error: "error",
    kBps: 'kBps',
    timeout: 'timeout',
    delayTimeout: 'delayTimeout',
    loadingTimeout: 'loadingTimeout',
    stats: 'stats',
    performance: "performance",
    record: 'record',
    recording: 'recording',
    recordingTimestamp: 'recordingTimestamp',
    recordStart: 'recordStart',
    recordEnd: 'recordEnd',
    recordCreateError: 'recordCreateError',
    buffer: 'buffer',
    videoFrame: 'videoFrame',
    start: 'start',
    metadata: 'metadata',
    resize: 'resize',
    streamEnd: 'streamEnd',
    streamSuccess: 'streamSuccess',
    streamMessage: 'streamMessage',
    streamError: 'streamError',
    volumechange: 'volumechange',
    destroy: 'destroy',
    mseSourceOpen: 'mseSourceOpen',
    mseSourceClose: 'mseSourceClose',
    mseSourceBufferError: 'mseSourceBufferError',
    mseSourceBufferBusy: 'mseSourceBufferBusy',
    videoWaiting: 'videoWaiting',
    videoTimeUpdate: 'videoTimeUpdate',
    videoSyncAudio: 'videoSyncAudio',
}


export const JESSIBUCA_EVENTS = {
    load: EVENTS.load,
    timeUpdate: EVENTS.timeUpdate,
    videoInfo: EVENTS.videoInfo,
    audioInfo: EVENTS.audioInfo,
    error: EVENTS.error,
    kBps: EVENTS.kBps,
    log: EVENTS.log,
    start: EVENTS.start,
    timeout: EVENTS.timeout,
    loadingTimeout: EVENTS.loadingTimeout,
    delayTimeout: EVENTS.delayTimeout,
    fullscreen: 'fullscreen',
    play: EVENTS.play,
    pause: EVENTS.pause,
    mute: EVENTS.mute,
    stats: EVENTS.stats,
    performance: EVENTS.performance,
    recordingTimestamp: EVENTS.recordingTimestamp,
    recordStart: EVENTS.recordStart,
    recordEnd: EVENTS.recordEnd
}

export const EVENTS_ERROR = {
    playError: 'playIsNotPauseOrUrlIsNull',
    fetchError: "fetchError",
    websocketError: 'websocketError',
    webcodecsH265NotSupport: 'webcodecsH265NotSupport',
    mediaSourceH265NotSupport: 'mediaSourceH265NotSupport',
    wasmDecodeError: 'wasmDecodeError'
}


export const WEBSOCKET_STATUS = {
    notConnect: 'notConnect',
    open: 'open',
    close: 'close',
    error: 'error'
}

export const BUFFER_STATUS = {
    empty: 'empty',
    buffering: 'buffering',
    full: 'full'
}


export const SCREENSHOT_TYPE = {
    download: 'download',
    base64: 'base64',
    blob: 'blob'
}


export const VIDEO_ENC_TYPE = {
    7: 'H264(AVC)', //
    12: 'H265(HEVC)' //
}

export const VIDEO_ENC_CODE = {
    h264: 7,
    h265: 12
}


export const AUDIO_ENC_TYPE = {
    10: 'AAC',
    7: 'ALAW',
    8: 'MULAW'
}

export const H265_NAL_TYPE = {
    vps: 32,
    sps: 33,
    pps: 34
}

export const CONTROL_HEIGHT = 38

export const SCALE_MODE_TYPE = {
    full: 0, //  视频画面完全填充canvas区域,画面会被拉伸
    auto: 1, // 视频画面做等比缩放后,高或宽对齐canvas区域,画面不被拉伸,但有黑边
    fullAuto: 2 // 视频画面做等比缩放后,完全填充canvas区域,画面不被拉伸,没有黑边,但画面显示不全
}


export const FILE_SUFFIX = {
    mp4: 'mp4',
    webm: 'webm'
};


export const CANVAS_RENDER_TYPE = {
    webcodecs: 'webcodecs',
    webgl: 'webgl',
    offscreen: 'offscreen'
}

export const ENCODED_VIDEO_TYPE = {
    key: 'key',
    delta: 'delta'
}


export const MP4_CODECS = {
    avc: 'video/mp4; codecs="avc1.64002A"',
    hev: 'video/mp4; codecs="hev1.1.6.L123.b0"',
}


export const MEDIA_SOURCE_STATE = {
    ended: 'ended',
    open: 'open',
    closed: 'closed'
}


// frag duration
export const FRAG_DURATION = Math.ceil(1000 / 25)


export const AUDIO_SYNC_VIDEO_DIFF = 200;
