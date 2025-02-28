export interface TimeRange {
    start: number;
    end: number;
    url: string;
    mediaStart: number;
    mediaDuration: number;
}
export interface TimelineOptions {
    timeRanges?: TimeRange[];
    onTimeUpdate?: (time: number) => void;
    onSeek?: (time: number) => void;
}
export interface TimelineBaseProps {
    currentTime: number;
    height?: number;
    backgroundColor?: string;
    cursorColor?: string;
    onTimeUpdate?: (time: number) => void;
    onSeek?: (time: number) => void;
}
export declare class TimelineBase {
    protected options: TimelineOptions;
    protected currentTime: number;
    protected currentMediaTime: number;
    constructor(options?: TimelineOptions);
    setTimeRanges(ranges: TimeRange[]): void;
    protected initializeTimeRanges(ranges: TimeRange[]): void;
    findTimeRangeForTime(time: number): TimeRange | undefined;
    originalToMediaTime(time: number): number;
    mediaToOriginalTime(time: number): number;
    getTotalDuration(): number;
    getTimeRangeExtent(): {
        start: number;
        end: number;
    };
    setCurrentTime(time: number, isMediaTime?: boolean): void;
    getCurrentTime(): number;
    getCurrentMediaTime(): number;
    seek(time: number, isMediaTime?: boolean): void;
    getTimeRanges(): TimeRange[];
    getGaps(): Array<{
        start: number;
        end: number;
    }>;
}
