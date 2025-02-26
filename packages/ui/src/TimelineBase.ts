export interface TimeRange {
  start: number;        // 原始时间轴上的开始时间
  end: number;          // 原始时间轴上的结束时间
  url: string;          // 对应的视频 URL
  mediaStart: number;   // MSE 媒体时间轴上的开始时间
  mediaDuration: number;// 在媒体时间轴上的持续时间
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

export class TimelineBase {
  protected options: TimelineOptions;
  protected currentTime: number = 0;
  protected currentMediaTime: number = 0;

  constructor(options: TimelineOptions = {}) {
    this.options = {
      timeRanges: [],
      ...options
    };
  }

  public setTimeRanges(ranges: TimeRange[]): void {
    this.options.timeRanges = ranges;
    this.initializeTimeRanges(ranges);
  }

  protected initializeTimeRanges(ranges: TimeRange[]): void {
    let currentMediaStart = 0;
    ranges.forEach(range => {
      range.mediaStart = currentMediaStart;
      range.mediaDuration = range.end - range.start;
      currentMediaStart += range.mediaDuration;
    });
  }

  public findTimeRangeForTime(time: number): TimeRange | undefined {
    return this.options.timeRanges?.find(range =>
      time >= range.start && time <= range.end
    );
  }

  public originalToMediaTime(time: number): number {
    const range = this.findTimeRangeForTime(time);
    if (!range) {
      const nextRange = this.options.timeRanges?.find(r => time < r.start);
      if (nextRange) return nextRange.mediaStart;
      const prevRange = [...(this.options.timeRanges || [])].reverse().find(r => time > r.end);
      if (prevRange) return prevRange.mediaStart + prevRange.mediaDuration;
      return 0;
    }
    return range.mediaStart + (time - range.start);
  }

  public mediaToOriginalTime(time: number): number {
    if (!this.options.timeRanges?.length) return time;

    for (const range of this.options.timeRanges) {
      if (time >= range.mediaStart && time < range.mediaStart + range.mediaDuration) {
        return range.start + (time - range.mediaStart);
      }
    }
    return this.options.timeRanges[0].start;
  }

  public getTotalDuration(): number {
    return this.options.timeRanges?.reduce((acc, range) => acc + range.mediaDuration, 0) || 0;
  }

  public getTimeRangeExtent(): { start: number; end: number; } {
    if (!this.options.timeRanges?.length) return { start: 0, end: 0 };
    return {
      start: Math.min(...this.options.timeRanges.map(r => r.start)),
      end: Math.max(...this.options.timeRanges.map(r => r.end))
    };
  }

  public setCurrentTime(time: number, isMediaTime: boolean = false): void {
    if (isMediaTime) {
      this.currentMediaTime = time;
      this.currentTime = this.mediaToOriginalTime(time);
    } else {
      this.currentTime = time;
      this.currentMediaTime = this.originalToMediaTime(time);
    }
    this.options.onTimeUpdate?.(this.currentTime);
  }

  public getCurrentTime(): number {
    return this.currentTime;
  }

  public getCurrentMediaTime(): number {
    return this.currentMediaTime;
  }

  public seek(time: number, isMediaTime: boolean = false): void {
    this.setCurrentTime(time, isMediaTime);
    this.options.onSeek?.(this.currentTime);
  }

  public getTimeRanges(): TimeRange[] {
    return this.options.timeRanges || [];
  }

  public getGaps(): Array<{ start: number; end: number; }> {
    const gaps: Array<{ start: number; end: number; }> = [];
    const ranges = this.options.timeRanges || [];

    for (let i = 0; i < ranges.length - 1; i++) {
      const gap = ranges[i + 1].start - ranges[i].end;
      if (gap > 0) {
        gaps.push({
          start: ranges[i].end,
          end: ranges[i + 1].start
        });
      }
    }
    return gaps;
  }
} 