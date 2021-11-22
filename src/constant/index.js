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
    videoBuffer: 0.5,
    vod: false,
    isResize: true,
    isFullResize: false,
    isFlv: false,
    debug: false,
    timeout: 30,
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
        recorder: false
    },
    hasControl: false,
    loadingText: '',
    background: '',
    decoder: 'decoder.js',
    url: '',//
    rotate: 0,
    text: '',
    forceNoOffscreen: true, // 默认是不采用
    hiddenAutoPause: false,
    protocol: PLAYER_PLAY_PROTOCOL.fetch,
    demuxType: DEMUX_TYPE.flv, //
    useWCS: false, //
    useEMS: false, //
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
    fullscreen: 'fullscreen',
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
    stats: 'stats',
    performance: "performance",
    record: 'record',
    recording: 'recording',
    recordingTimestamp: 'recordingTimestamp',
    buffer: 'buffer',
    videoFrame: 'videoFrame',
    start: 'start',
    metadata: 'metadata',
    resize: 'resize',
    streamRate: 'streamRate',
    streamEnd: 'streamEnd',
    streamSuccess:'streamSuccess',
    streamMessage: 'streamMessage',
    streamError: 'streamError',
    volumechange: 'volumechange',
    frameStart: 'frameStart'
}


export const JESSIBUCA_EVENTS = {
    load: EVENTS.load,
    timeUpdate: EVENTS.timeUpdate,
    videoInfo: EVENTS.videoInfo,
    audioInfo: EVENTS.audioInfo,
    error: EVENTS.error,
    kBps: EVENTS.kBps,
    log: EVENTS.log,
    start: EVENTS.frameStart,
    timeout: EVENTS.timeout,
    fullscreen: EVENTS.fullscreen,
    play: EVENTS.play,
    pause: EVENTS.pause,
    mute: EVENTS.mute,
    stats: EVENTS.stats,
    performance: EVENTS.performance
}

export const EVENTS_ERROR = {
    fetchError: "fetchError",
    websocketError: 'websocketError',
    websocketCloseSuccess: 'websocketCloseSuccess',
    websocketClosedByError: 'websocketClosedByError',
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
    7: 'H264', //
    12: 'H265' //
}

export const AUDIO_ENC_TYPE = {
    10: 'AAC',
    7: 'ALAW',
    8: 'MULAW'
}

export const VIDEO_ENC_TYPE_OBJ = {
    h264: 'H264',
    H265: 'H265'
}

export const CONTROL_HEIGHT = 38

export const SCALE_MODE_TYPE = {
    full: 0, //  视频画面完全填充canvas区域,画面会被拉伸
    auto: 1, // 视频画面做等比缩放后,高或宽对齐canvas区域,画面不被拉伸,但有黑边
    fullAuto: 2 // 视频画面做等比缩放后,完全填充canvas区域,画面不被拉伸,没有黑边,但画面显示不全
}
