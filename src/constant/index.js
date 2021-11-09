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
    isFullSize: false,
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
    decode: 'decode'
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
    streamError: 'streamError',
    volumechange: 'volumechange'
}

export const EVENTS_ERROR = {
    fetchError: "fetchError",
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
    7: 'H264', // 对应jessibuca的 7
    12: 'H265' // 对应jessibuca的 12
}

export const VIDEO_ENC_TYPE_OBJ = {
    h264: 'H264',
    H265: 'H265'
}
