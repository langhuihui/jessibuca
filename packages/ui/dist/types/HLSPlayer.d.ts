import { TimeRange } from './TimelineBase';
import { SlidingWindowConfig } from 'jv4-demuxer';
export type { TimeRange };
export interface HLSPlayerOptions {
    showPlaybackRate?: boolean;
    showProgress?: boolean;
    playbackRates?: number[];
    autoGenerateUI?: boolean;
    timeRangeMode?: boolean;
    timeRanges?: TimeRange[];
    debug?: {
        enabled?: boolean;
        showTimeRanges?: boolean;
        showMediaTimeline?: boolean;
    };
}
export declare class HLSPlayer {
    private demuxer;
    private options;
    private videoElement;
    private controlsElement?;
    private progressInput?;
    private progressCanvas?;
    private timeDisplay?;
    private playButton?;
    private rateButtons;
    private updateProgressInterval?;
    private currentTimeRange?;
    private currentMediaTime;
    private debugContainer?;
    private debugTimelineCanvas?;
    private debugMediaCanvas?;
    private totalPlaylistDuration;
    private loadingIndicator?;
    private container?;
    private progressInfoLabel?;
    private static lastProgressLogTime;
    private static lastTimelineLogTime;
    constructor(videoElement: HTMLVideoElement, options?: HLSPlayerOptions);
    /**
     * Setup event listeners for demuxer to handle various events
     */
    private setupDemuxerListeners;
    private initializeTimeRanges;
    private handleTimeUpdate;
    /**
     * 查找指定时间所属的时间范围
     */
    private findTimeRangeForTime;
    private switchToTimeRange;
    private originalToMediaTime;
    private mediaToOriginalTime;
    private createUI;
    private createDebugUI;
    private updateTimeRangesDisplay;
    private calculateSeekTime;
    private handleSeek;
    private updateProgress;
    private formatTime;
    private togglePlay;
    private startProgressUpdate;
    private stopProgressUpdate;
    load(url: string): Promise<void>;
    play(): Promise<void>;
    pause(): void;
    /**
     * 跳转到指定时间
     */
    seek(time: number): void;
    /**
     * 获取当前播放时间
     */
    getCurrentTime(): number;
    /**
     * 获取总时长
     */
    getDuration(): number;
    /**
     * 设置播放速率
     */
    setPlaybackRate(rate: number): void;
    /**
     * 销毁播放器实例
     */
    destroy(): void;
    getPlaybackRate(): number;
    getOptions(): HLSPlayerOptions;
    setTimeRanges(ranges: TimeRange[]): void;
    private updateProgressDisplay;
    private updateDebugDisplay;
    private updateTimelineDebugCanvas;
    private updateMediaDebugCanvas;
    forceUpdateProgress(): void;
    /**
     * 记录日志信息
     */
    private log;
    /**
     * 配置滑动窗口参数
     */
    setSlidingWindowConfig(config: Partial<SlidingWindowConfig>): void;
    /**
     * 获取当前滑动窗口配置
     */
    getSlidingWindowConfig(): SlidingWindowConfig | undefined;
    /**
     * 获取视频片段总数
     */
    getTotalSegments(): number;
    /**
     * 检查播放器是否正在播放
     */
    isPlaying(): boolean;
}
