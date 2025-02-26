import { HLSv7Demuxer } from 'jv4-demuxer';
import { h, render } from 'vue';
import Timeline from './Timeline';
import { TimeRange } from './TimelineBase';
import './HLSPlayer.css';

export type { TimeRange };

export interface HLSPlayerOptions {
  showPlaybackRate?: boolean;
  showProgress?: boolean;
  playbackRates?: number[];
  autoGenerateUI?: boolean;
  timeRangeMode?: boolean;  // 是否使用时间片段模式
  timeRanges?: TimeRange[]; // 可播放的时间片段数组
  debug?: {
    enabled?: boolean;
    showTimeRanges?: boolean;
    showMediaTimeline?: boolean;
  };
}

export class HLSPlayer {
  private demuxer: HLSv7Demuxer;
  private options: HLSPlayerOptions;
  private videoElement: HTMLVideoElement;
  private controlsElement?: HTMLDivElement;
  private progressInput?: HTMLInputElement;
  private progressCanvas?: HTMLCanvasElement;
  private timeDisplay?: HTMLSpanElement;
  private playButton?: HTMLButtonElement;
  private rateButtons: HTMLButtonElement[] = [];
  private updateProgressInterval?: number;
  private currentTimeRange?: TimeRange;
  private currentMediaTime: number = 0;
  private debugContainer?: HTMLDivElement;
  private debugTimelineCanvas?: HTMLCanvasElement;
  private debugMediaCanvas?: HTMLCanvasElement;
  private totalPlaylistDuration: number = 0;

  constructor(videoElement: HTMLVideoElement, options: HLSPlayerOptions = {}) {
    this.videoElement = videoElement;
    this.options = {
      showPlaybackRate: true,
      showProgress: true,
      playbackRates: [0.5, 1, 1.5, 2],
      autoGenerateUI: true,
      timeRangeMode: false,
      timeRanges: [],
      ...options
    };
    this.demuxer = new HLSv7Demuxer(videoElement);

    if (this.options.autoGenerateUI) {
      this.createUI();
    }

    // 初始化时间片段
    if (this.options.timeRanges?.length) {
      this.initializeTimeRanges(this.options.timeRanges);
    }

    // 监听视频时间更新
    this.videoElement.addEventListener('timeupdate', this.handleTimeUpdate.bind(this));
  }

  private initializeTimeRanges(ranges: TimeRange[]): void {
    let currentMediaStart = 0;
    ranges.forEach(range => {
      range.mediaStart = currentMediaStart;
      range.mediaDuration = range.end - range.start;
      currentMediaStart += range.mediaDuration;
    });
  }

  private handleTimeUpdate(): void {
    if (!this.options.timeRangeMode) return;

    this.currentMediaTime = this.videoElement.currentTime;
    const originalTime = this.mediaToOriginalTime(this.currentMediaTime);

    // 检查是否需要切换片段
    const targetRange = this.findTimeRangeForTime(originalTime);
    if (targetRange && targetRange !== this.currentTimeRange) {
      this.switchToTimeRange(targetRange);
    }
  }

  private findTimeRangeForTime(time: number): TimeRange | undefined {
    return this.options.timeRanges?.find(range =>
      time >= range.start && time <= range.end
    );
  }

  private async switchToTimeRange(range: TimeRange): Promise<void> {
    this.currentTimeRange = range;
    const relativeTime = this.currentMediaTime - range.mediaStart;
    await this.load(range.url);
    this.seek(range.start + relativeTime);
  }

  private originalToMediaTime(time: number): number {
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

  private mediaToOriginalTime(time: number): number {
    if (!this.options.timeRanges?.length) return time;

    for (const range of this.options.timeRanges) {
      if (time >= range.mediaStart && time < range.mediaStart + range.mediaDuration) {
        return range.start + (time - range.mediaStart);
      }
    }
    return this.options.timeRanges[0].start;
  }

  private createUI(): void {
    const container = document.createElement('div');
    container.className = 'hls-player';
    this.videoElement.parentNode?.insertBefore(container, this.videoElement);
    container.appendChild(this.videoElement);
    this.videoElement.className = 'hls-player-video';

    this.controlsElement = document.createElement('div');
    this.controlsElement.className = 'hls-player-controls';
    container.appendChild(this.controlsElement);

    this.playButton = document.createElement('button');
    this.playButton.textContent = '播放';
    this.playButton.onclick = this.togglePlay.bind(this);
    this.controlsElement.appendChild(this.playButton);

    if (this.options.showProgress) {
      const progressContainer = document.createElement('div');
      progressContainer.className = 'hls-player-progress';

      // 创建进度条容器，包含 canvas 和 input
      const progressWrapper = document.createElement('div');
      progressWrapper.className = 'progress-wrapper';

      // 创建 canvas 用于绘制时间片段
      this.progressCanvas = document.createElement('canvas');
      this.progressCanvas.className = 'progress-canvas';
      progressWrapper.appendChild(this.progressCanvas);

      this.progressInput = document.createElement('input');
      this.progressInput.type = 'range';
      this.progressInput.min = '0';
      this.progressInput.max = '0';
      this.progressInput.value = '0';
      this.progressInput.setAttribute('aria-label', '视频进度条');
      this.progressInput.oninput = this.handleSeek.bind(this);
      progressWrapper.appendChild(this.progressInput);

      progressContainer.appendChild(progressWrapper);

      this.timeDisplay = document.createElement('span');
      this.timeDisplay.textContent = '0:00 / 0:00';
      progressContainer.appendChild(this.timeDisplay);

      this.controlsElement.appendChild(progressContainer);

      // 初始化时间片段显示
      if (this.options.timeRangeMode) {
        this.updateTimeRangesDisplay();
      }
    }

    if (this.options.showPlaybackRate) {
      const rateContainer = document.createElement('div');
      rateContainer.className = 'hls-player-rate';

      this.options.playbackRates?.forEach(rate => {
        const button = document.createElement('button');
        button.textContent = `${rate}x`;
        button.onclick = () => this.setPlaybackRate(rate);
        button.className = rate === 1 ? 'rate-active' : '';
        this.rateButtons.push(button);
        rateContainer.appendChild(button);
      });

      this.controlsElement.appendChild(rateContainer);
    }

    if (this.options.debug?.enabled) {
      this.createDebugUI();
    }

    this.startProgressUpdate();
  }

  private createDebugUI(): void {
    this.debugContainer = document.createElement('div');
    this.debugContainer.className = 'hls-player-debug';

    // Create timeline component using Vue render function
    const timelineVNode = h(Timeline, {
      timeRanges: this.options.timeRanges,
      showOriginalTimeline: this.options.debug?.showTimeRanges,
      showMediaTimeline: this.options.debug?.showMediaTimeline,
      height: 20,
      onTimeUpdate: (time: number) => {
        this.seek(time);
      },
      onSeek: (time: number) => {
        this.seek(time);
      }
    });

    // Create a container for the timeline
    const timelineContainer = document.createElement('div');
    render(timelineVNode, timelineContainer);
    this.debugContainer.appendChild(timelineContainer);

    const container = this.videoElement.closest('.hls-player');
    container?.appendChild(this.debugContainer);
  }

  private updateTimeRangesDisplay(): void {
    if (!this.progressCanvas || !this.options.timeRanges) return;

    const ctx = this.progressCanvas.getContext('2d');
    if (!ctx) return;

    // 设置 canvas 尺寸
    const canvas = this.progressCanvas;
    canvas.width = canvas.offsetWidth * window.devicePixelRatio;
    canvas.height = canvas.offsetHeight * window.devicePixelRatio;

    // 清除画布
    ctx.clearRect(0, 0, canvas.width, canvas.height);

    if (!this.options.timeRanges.length) return;

    // 计算总时间范围
    const totalStart = Math.min(...this.options.timeRanges.map(r => r.start));
    const totalEnd = Math.max(...this.options.timeRanges.map(r => r.end));
    const totalDuration = totalEnd - totalStart;

    // 绘制时间片段
    ctx.fillStyle = 'rgba(0, 160, 255, 0.3)';
    this.options.timeRanges.forEach(range => {
      const startX = ((range.start - totalStart) / totalDuration) * canvas.width;
      const width = ((range.end - range.start) / totalDuration) * canvas.width;
      ctx.fillRect(startX, 0, width, canvas.height);
    });
  }

  private calculateSeekTime(inputValue: number): { time: number; url: string; } {
    if (!this.options.timeRangeMode || !this.options.timeRanges?.length) {
      return { time: inputValue, url: '' };
    }

    const totalStart = Math.min(...this.options.timeRanges.map(r => r.start));
    const totalEnd = Math.max(...this.options.timeRanges.map(r => r.end));
    const targetTime = totalStart + (inputValue / 100) * (totalEnd - totalStart);

    const range = this.findTimeRangeForTime(targetTime);
    if (!range) {
      // 如果没找到对应的时间片段，找最近的一个
      const nextRange = this.options.timeRanges.find(r => r.start > targetTime);
      const prevRange = [...this.options.timeRanges].reverse().find(r => r.end < targetTime);
      const targetRange = nextRange || prevRange;
      if (targetRange) {
        return {
          time: nextRange ? nextRange.start : prevRange!.end,
          url: targetRange.url
        };
      }
      return { time: targetTime, url: '' };
    }

    return {
      time: targetTime,
      url: range.url
    };
  }

  private handleSeek(e: Event): void {
    const target = e.target as HTMLInputElement;
    const value = Number(target.value);

    if (this.options.timeRangeMode) {
      const { time, url } = this.calculateSeekTime(value);
      if (url && url !== this.currentTimeRange?.url) {
        this.load(url).then(() => this.seek(time));
      } else {
        this.seek(time);
      }
    } else {
      this.seek(value);
    }
  }

  private updateProgress(): void {
    if (!this.options.showProgress) return;

    const currentTime = this.getCurrentTime();
    const duration = this.getDuration();

    // Make sure we have valid duration to prevent NaN calculations
    const validDuration = isNaN(duration) || duration <= 0 || !isFinite(duration) ? 100 : duration;

    if (this.options.timeRangeMode && this.options.timeRanges?.length) {
      const totalStart = Math.min(...this.options.timeRanges.map(r => r.start));
      const totalEnd = Math.max(...this.options.timeRanges.map(r => r.end));
      const range = this.findTimeRangeForTime(currentTime);

      if (range) {
        this.currentTimeRange = range;
        if (this.progressInput) {
          this.progressInput.value = String(((currentTime - totalStart) / (totalEnd - totalStart)) * 100);
          this.progressInput.max = '100';
        }
        if (this.timeDisplay) {
          this.timeDisplay.textContent = `${this.formatTime(currentTime - totalStart)} / ${this.formatTime(totalEnd - totalStart)}`;
        }
      }
    } else {
      if (this.progressInput) {
        // Ensure current time is within bounds
        const boundedCurrentTime = Math.min(currentTime, validDuration);
        
        const percentage = (boundedCurrentTime / validDuration) * 100;
        // Ensure the percentage is within valid bounds (0-100)
        const normalizedPercentage = Math.min(100, Math.max(0, isNaN(percentage) ? 0 : percentage));
        
        // Use percentage-based value to avoid issues with non-standard durations
        this.progressInput.max = '100';
        this.progressInput.value = String(normalizedPercentage);
        
        // Limit debug logging frequency to avoid console spam
        if (Math.floor(currentTime) % 5 === 0) { // Log every 5 seconds
          console.log(`Progress: ${boundedCurrentTime.toFixed(2)}s / ${validDuration.toFixed(2)}s (${normalizedPercentage.toFixed(2)}%)`);
        }
      }
      
      if (this.timeDisplay) {
        // Format and display the time
        this.timeDisplay.textContent = `${this.formatTime(currentTime)} / ${this.formatTime(validDuration)}`;
      }
    }

    if (this.playButton) {
      this.playButton.textContent = this.isPlaying() ? '暂停' : '播放';
    }
  }

  private formatTime(seconds: number): string {
    const mins = Math.floor(seconds / 60);
    const secs = Math.floor(seconds % 60);
    return `${mins}:${secs.toString().padStart(2, '0')}`;
  }

  private async togglePlay(): Promise<void> {
    if (this.isPlaying()) {
      this.pause();
    } else {
      await this.play();
    }
  }

  private startProgressUpdate(): void {
    this.updateProgressInterval = window.setInterval(() => {
      this.updateProgress();
      if (this.options.debug?.enabled) {
        this.updateDebugDisplay();
      }
    }, 1000 / 30); // 更新频率提高到 30fps
  }

  private stopProgressUpdate(): void {
    if (this.updateProgressInterval) {
      clearInterval(this.updateProgressInterval);
    }
  }

  public async load(url: string): Promise<void> {
    await this.demuxer.load(url);
    
    // Calculate the total playlist duration after the demuxer has loaded the m3u8
    try {
      // Allow time for the video metadata to load
      await new Promise(resolve => {
        const checkDuration = () => {
          if (this.videoElement.duration > 0 && !isNaN(this.videoElement.duration)) {
            resolve(true);
          } else {
            setTimeout(checkDuration, 100); // Check every 100ms
          }
        };
        
        // Start checking
        checkDuration();
        
        // Also listen for loadedmetadata event as a fallback
        const onMetadataLoaded = () => {
          this.videoElement.removeEventListener('loadedmetadata', onMetadataLoaded);
          resolve(true);
        };
        this.videoElement.addEventListener('loadedmetadata', onMetadataLoaded);
      });
      
      // Method 1: Get m3u8 segment information from the demuxer
      if ((this.demuxer as any).playlistInfo && Array.isArray((this.demuxer as any).playlistInfo.segments)) {
        const segments = (this.demuxer as any).playlistInfo.segments;
        
        // Calculate total duration by summing segment durations
        let calculatedDuration = 0;
        let validSegmentCount = 0;
        
        segments.forEach(segment => {
          if (segment.duration && isFinite(segment.duration) && segment.duration > 0) {
            calculatedDuration += segment.duration;
            validSegmentCount++;
          }
        });
        
        if (calculatedDuration > 0 && isFinite(calculatedDuration)) {
          this.totalPlaylistDuration = calculatedDuration;
          console.log(`Total playlist duration calculated: ${this.totalPlaylistDuration.toFixed(2)}s from ${validSegmentCount} segments`);
        } else if (segments.length > 0) {
          // If we couldn't calculate from durations, try to estimate based on segment count
          // Try to fetch the playlist directly for parsing durations
          try {
            await this.fetchAndParsePlaylist(url);
          } catch (err) {
            console.warn('Failed to fetch and parse playlist directly:', err);
            
            // Fall back to estimating duration based on segment count
            const estimatedDuration = isFinite(this.videoElement.duration) && this.videoElement.duration > 0 
              ? this.videoElement.duration 
              : segments.length * 4; // Assume average 4 seconds per segment as fallback
            
            this.totalPlaylistDuration = estimatedDuration;
            console.log(`Estimated playlist duration: ${this.totalPlaylistDuration.toFixed(2)}s (based on ${segments.length} segments)`);
          }
        } else {
          // Try direct parsing if no valid segments found
          try {
            await this.fetchAndParsePlaylist(url);
          } catch (err) {
            console.warn('Failed to fetch and parse playlist directly:', err);
            // Fallback to a reasonable default if we can't calculate
            this.totalPlaylistDuration = 60; // Default 60s if we can't determine
            console.log(`Using default duration: ${this.totalPlaylistDuration.toFixed(2)}s (could not calculate)`);
          }
        }
      } else {
        // Method 2: Try to fetch and parse the playlist directly
        try {
          await this.fetchAndParsePlaylist(url);
        } catch (err) {
          console.warn('Failed to fetch and parse playlist directly:', err);
          
          // Fallback to video element duration or default
          this.totalPlaylistDuration = isFinite(this.videoElement.duration) && this.videoElement.duration > 0 
            ? this.videoElement.duration 
            : 60; // Default 60s
          console.log(`Using video element duration: ${this.totalPlaylistDuration.toFixed(2)}s`);
        }
      }
      
      // Set up listeners to update duration as more segments are loaded
      this.setupDurationUpdateListeners();
      
    } catch (error) {
      console.error('Error calculating playlist duration:', error);
      // Fallback to a reasonable default
      this.totalPlaylistDuration = 60; // Default 60s if calculation fails
      console.log(`Using default duration due to error: ${this.totalPlaylistDuration.toFixed(2)}s`);
    }
  }
  
  private async fetchAndParsePlaylist(url: string): Promise<void> {
    try {
      // Fetch the m3u8 file directly
      const response = await fetch(url);
      if (!response.ok) {
        throw new Error(`Failed to fetch playlist: ${response.status} ${response.statusText}`);
      }
      
      const content = await response.text();
      console.log('Successfully fetched m3u8 content');
      
      // Parse the m3u8 content to extract segment durations
      const lines = content.split('\n');
      let totalDuration = 0;
      let segmentCount = 0;
      
      for (let i = 0; i < lines.length; i++) {
        const line = lines[i].trim();
        if (line.startsWith('#EXTINF:')) {
          // Extract duration from the EXTINF tag
          // Format is typically #EXTINF:duration,optional-title
          const durationPart = line.split(':')[1].split(',')[0];
          const duration = parseFloat(durationPart);
          
          if (!isNaN(duration) && isFinite(duration) && duration > 0) {
            totalDuration += duration;
            segmentCount++;
          }
        }
      }
      
      if (totalDuration > 0 && segmentCount > 0) {
        this.totalPlaylistDuration = totalDuration;
        console.log(`Duration calculated from m3u8 parsing: ${this.totalPlaylistDuration.toFixed(2)}s from ${segmentCount} segments`);
      } else {
        throw new Error('No valid durations found in m3u8 content');
      }
    } catch (error) {
      console.error('Error parsing m3u8 content:', error);
      throw error; // Re-throw for the caller to handle
    }
  }
  
  private setupDurationUpdateListeners(): void {
    // Listen for buffered ranges updates to improve duration calculation
    const updateDurationFromBuffer = () => {
      if (this.videoElement.buffered.length > 0) {
        const bufferedEnd = this.videoElement.buffered.end(this.videoElement.buffered.length - 1);
        if (bufferedEnd > 0 && isFinite(bufferedEnd) && bufferedEnd > this.totalPlaylistDuration) {
          this.totalPlaylistDuration = bufferedEnd;
          console.log(`Updated duration from buffer: ${this.totalPlaylistDuration}s`);
        }
      }
    };
    
    // Update when more content is buffered
    this.videoElement.addEventListener('progress', updateDurationFromBuffer);
    
    // Also update when seeking near the end
    this.videoElement.addEventListener('seeking', () => {
      const currentTime = this.videoElement.currentTime;
      if (currentTime > 0 && isFinite(currentTime) && currentTime > this.totalPlaylistDuration * 0.9) {
        // If seeking near the end, update duration if needed
        updateDurationFromBuffer();
      }
    });
  }

  public async play(): Promise<void> {
    await this.demuxer.play();
    this.updateProgress();
  }

  public pause(): void {
    this.demuxer.pause();
    this.updateProgress();
  }

  public seek(time: number): void {
    if (this.options.timeRangeMode) {
      const mediaTime = this.originalToMediaTime(time);
      this.videoElement.currentTime = mediaTime;
    } else {
      this.videoElement.currentTime = time;
    }
  }

  public setPlaybackRate(rate: number): void {
    this.demuxer.setPlaybackRate(rate);
    if (this.options.autoGenerateUI) {
      this.rateButtons.forEach(button => {
        button.className = button.textContent === `${rate}x` ? 'rate-active' : '';
      });
    }
  }

  public destroy(): void {
    this.stopProgressUpdate();
    this.demuxer.destroy();
    if (this.options.autoGenerateUI) {
      const container = this.videoElement.closest('.hls-player');
      if (container && container.parentNode) {
        container.parentNode.insertBefore(this.videoElement, container);
        container.remove();
      }
    }
    this.videoElement.removeEventListener('timeupdate', this.handleTimeUpdate.bind(this));
  }

  public getCurrentTime(): number {
    if (this.options.timeRangeMode) {
      return this.mediaToOriginalTime(this.videoElement.currentTime);
    }
    return this.videoElement.currentTime;
  }

  public getDuration(): number {
    if (this.options.timeRangeMode) {
      return this.options.timeRanges?.reduce((acc, range) => acc + range.mediaDuration, 0) || 0;
    }
    
    // Make sure we never return Infinity or NaN
    if (!isFinite(this.totalPlaylistDuration) || this.totalPlaylistDuration <= 0) {
      // If we couldn't calculate a valid duration, use a reasonable default
      // or try to get it from the video element if available
      return isFinite(this.videoElement.duration) && this.videoElement.duration > 0 
        ? this.videoElement.duration 
        : 60; // Default 60s
    }
    
    return this.totalPlaylistDuration;
  }

  public isPlaying(): boolean {
    return !this.videoElement.paused;
  }

  public getPlaybackRate(): number {
    return this.videoElement.playbackRate;
  }

  public getOptions(): HLSPlayerOptions {
    return this.options;
  }

  public setTimeRanges(ranges: TimeRange[]): void {
    this.options.timeRanges = ranges;
    this.options.timeRangeMode = true;
    this.initializeTimeRanges(ranges);

    if (this.options.autoGenerateUI) {
      this.updateTimeRangesDisplay();
      if (this.options.debug?.enabled) {
        // 重新创建 debug UI 以更新时间轴
        if (this.debugContainer) {
          const container = this.videoElement.closest('.hls-player');
          container?.removeChild(this.debugContainer);
        }
        this.createDebugUI();
      }
    }
  }

  private updateProgressDisplay(): void {
    if (!this.progressCanvas || !this.options.timeRanges) return;
    const ctx = this.progressCanvas.getContext('2d');
    if (!ctx) return;

    const canvas = this.progressCanvas;
    // 设置 canvas 尺寸
    canvas.width = canvas.offsetWidth * window.devicePixelRatio;
    canvas.height = canvas.offsetHeight * window.devicePixelRatio;

    // 清除画布
    ctx.clearRect(0, 0, canvas.width, canvas.height);

    if (!this.options.timeRanges.length) return;

    // 计算总媒体时长
    const totalDuration = this.options.timeRanges.reduce(
      (acc, range) => acc + range.mediaDuration,
      0
    );

    // 绘制时间片段
    ctx.fillStyle = 'rgba(0, 160, 255, 0.3)';
    this.options.timeRanges.forEach(range => {
      const startX = (range.mediaStart / totalDuration) * canvas.width;
      const width = (range.mediaDuration / totalDuration) * canvas.width;
      ctx.fillRect(startX, 0, width, canvas.height);
    });

    // 绘制当前位置
    if (this.currentMediaTime > 0) {
      ctx.fillStyle = '#18a058';
      const x = (this.currentMediaTime / totalDuration) * canvas.width;
      ctx.fillRect(x - 1, 0, 2, canvas.height);
    }
  }

  private updateDebugDisplay(): void {
    if (!this.options.debug?.enabled) return;

    if (this.options.debug?.showTimeRanges) {
      this.updateTimelineDebugCanvas();
    }

    if (this.options.debug?.showMediaTimeline) {
      this.updateMediaDebugCanvas();
    }
  }

  private updateTimelineDebugCanvas(): void {
    if (!this.debugTimelineCanvas || !this.options.timeRanges?.length) return;

    const canvas = this.debugTimelineCanvas;
    const ctx = canvas.getContext('2d');
    if (!ctx) return;

    // Set canvas size
    canvas.width = canvas.offsetWidth * window.devicePixelRatio;
    canvas.height = canvas.offsetHeight * window.devicePixelRatio;

    // Clear canvas
    ctx.clearRect(0, 0, canvas.width, canvas.height);

    // Calculate total range
    const totalStart = Math.min(...this.options.timeRanges.map(r => r.start));
    const totalEnd = Math.max(...this.options.timeRanges.map(r => r.end));
    const totalDuration = totalEnd - totalStart;

    // Draw gaps
    ctx.fillStyle = 'rgba(0, 0, 0, 0.1)';
    for (let i = 0; i < this.options.timeRanges.length - 1; i++) {
      const gapStart = this.options.timeRanges[i].end;
      const gapEnd = this.options.timeRanges[i + 1].start;
      const startX = ((gapStart - totalStart) / totalDuration) * canvas.width;
      const width = ((gapEnd - gapStart) / totalDuration) * canvas.width;
      ctx.fillRect(startX, 0, width, canvas.height);
    }

    // Draw segments
    ctx.fillStyle = 'rgba(0, 160, 255, 0.3)';
    this.options.timeRanges.forEach(range => {
      const startX = ((range.start - totalStart) / totalDuration) * canvas.width;
      const width = ((range.end - range.start) / totalDuration) * canvas.width;
      ctx.fillRect(startX, 0, width, canvas.height);
    });

    // Draw current position
    const currentTime = this.getCurrentTime();
    ctx.fillStyle = '#18a058';
    const x = ((currentTime - totalStart) / totalDuration) * canvas.width;
    ctx.fillRect(x - 1, 0, 2, canvas.height);
  }

  private updateMediaDebugCanvas(): void {
    if (!this.debugMediaCanvas || !this.options.timeRanges?.length) return;

    const canvas = this.debugMediaCanvas;
    const ctx = canvas.getContext('2d');
    if (!ctx) return;

    // Set canvas size
    canvas.width = canvas.offsetWidth * window.devicePixelRatio;
    canvas.height = canvas.offsetHeight * window.devicePixelRatio;

    // Clear canvas
    ctx.clearRect(0, 0, canvas.width, canvas.height);

    const totalDuration = this.getDuration();

    // Draw segments
    ctx.fillStyle = 'rgba(0, 160, 255, 0.3)';
    this.options.timeRanges.forEach(range => {
      const startX = (range.mediaStart / totalDuration) * canvas.width;
      const width = (range.mediaDuration / totalDuration) * canvas.width;
      ctx.fillRect(startX, 0, width, canvas.height);
    });

    // Draw current position
    const mediaTime = this.videoElement.currentTime;
    ctx.fillStyle = '#18a058';
    const x = (mediaTime / totalDuration) * canvas.width;
    ctx.fillRect(x - 1, 0, 2, canvas.height);
  }
} 