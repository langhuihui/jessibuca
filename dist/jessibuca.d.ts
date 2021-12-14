declare namespace Jessibuca {

    interface Config {
        container: HTMLMediaElement | string

        videoBuffer?: number,
        decoder?: string,
        forceNoOffscreen?: boolean,
        hiddenAutoPause?: boolean,
        hasAudio?: boolean,
        rotate?: boolean,
        isResize?: boolean,
        isFullSize?: boolean,
        isFlv?: boolean,
        debug?: boolean,
        timeout?: number,
        supportDblclickFullscreen?: boolean,
        showBandwidth?: boolean,
        operateBtns?: {
            fullscreen?: boolean,
            screenshot?: boolean,
            play?: boolean,
            audio?: boolean
            record?: boolean
        }
        keepScreenOn?: boolean,
        isNotMute?: boolean,
        loadingText?: boolean,
        background?: string,
        useWCS?: boolean,
        useMSE?: boolean
    }

    interface JessibucaConstructor {
        new(config?: Config): Jessibuca
    }

    interface Jessibuca {
        constructor: JessibucaConstructor,

        setDebug(flag: boolean): void,

        mute(): void,

        cancelMute(): void,

        audioResume(): void,

        setTimeout(): void,

        setScaleMode(mode: number): void,

        pause(): Promise<void>,

        close(): void,

        destroy(): void,

        clearView(): void,

        play(url?: string): Promise<void>,

        resize(): void,

        setBufferTime(time: number): void,

        setRotate(deg: number): void,

        setVolume(volume: number): void,

        hasLoaded(): boolean,

        setKeepScreenOn(): boolean,

        setFullscreen(flag: boolean): void,

        screenshot(filename?: string, format?: string, quality?: number, type?: string)

        isPlaying(): boolean,

        isMute(): boolean,

        isRecording(): boolean,

        on(event: string, callback: Function): void

    }
}
