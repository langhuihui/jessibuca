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
    showBandwidth: false,
    keepScreenOn: false,
    isNotMute: false,
    hasAudio: true,
    operateBtns: {
        fullscreen: false,
        screenshot: false,
        play: false,
        audio: false
    },
    loadingText: '',
    background: '',
    decoder: 'decoder.js',
    rotate: 0,
    text: '',
    forceNoOffscreen: true,
    hiddenAutoPause: false
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
    audio: 0,
    video: 1
}

export const WORKER_SEND_TYPE = {
    init: 'init',
    decode: 'decode',
    audioDecode: 'audioDecode',
    videoDecode: 'videoDecode',
    close: 'close'
}

//
export const EVEMTS = {
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
    resize: 'resize'
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
