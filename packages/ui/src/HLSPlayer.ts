import { HLSv7Demuxer } from 'jv4-demuxer';
import { h, render } from 'vue';
import Timeline from './Timeline';
import { TimeRange } from './TimelineBase';
import './HLSPlayer.css';
import { SegmentLoader, VirtualTimeline, MediaSegment, BufferRange, SlidingWindowConfig } from 'jv4-demuxer';

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

interface PlaylistSegment {
  duration: number;
  time: Date;
  url: string;
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
  private loadingIndicator?: HTMLDivElement;
  private container?: HTMLDivElement;
  private progressInfoLabel?: HTMLDivElement;

  // 静态属性，用于记录上次日志输出时间
  private static lastProgressLogTime = 0;

  // 静态属性，用于记录上次时间轴渲染日志输出时间
  private static lastTimelineLogTime = 0;

  constructor(videoElement: HTMLVideoElement, options: HLSPlayerOptions = {}) {
    this.videoElement = videoElement;
    this.options = {
      showPlaybackRate: true,
      showProgress: true,
      playbackRates: [0.5, 1, 1.5, 2, 3],
      autoGenerateUI: true,
      timeRangeMode: false,
      timeRanges: [],
      ...options
    };

    // Create the HLSv7Demuxer instance with optimized configuration
    this.demuxer = new HLSv7Demuxer(this.videoElement, {
      logLevel: (options.debug?.enabled ? 'debug' : 'info'),
      // Configure sliding window for efficient segment management
      slidingWindow: {
        enabled: true,
        forward: 2,  // Preload 2 segments ahead
        backward: 1  // Keep 1 segment in history
      }
    });

    // Setup event listeners for demuxer
    this.setupDemuxerListeners();

    // Create UI if enabled
    if (this.options.autoGenerateUI) {
      this.createUI();
    }

    // Create debug UI if enabled
    if (this.options.debug?.enabled) {
      this.createDebugUI();
    }

    // Initialize time ranges if provided
    if (this.options.timeRanges?.length) {
      this.initializeTimeRanges(this.options.timeRanges);
    }

    // Setup video element event listeners
    this.videoElement.addEventListener('timeupdate', this.handleTimeUpdate.bind(this));

    // 启动进度更新
    if (this.options.showProgress) {
      this.startProgressUpdate();
    }
  }

  /**
   * Setup event listeners for demuxer to handle various events
   */
  private setupDemuxerListeners(): void {
    // Listen for playlist updates
    this.demuxer.on('playlistUpdate', (playlist) => {
      this.log(`播放列表更新: 片段数=${playlist.length}`, 'info');

      // Calculate total duration
      this.totalPlaylistDuration = this.demuxer.getTotalDuration();

      this.log(`总时长更新: ${this.totalPlaylistDuration}秒`, 'info');

      // Update progress display
      this.updateProgress();
    });

    // Listen for buffer updates
    this.demuxer.on('bufferUpdate', (ranges: BufferRange[]) => {
      // Update buffer visualization if in debug mode
      if (this.options.debug?.enabled) {
        this.updateDebugDisplay();
      }
    });

    // Listen for segment loaded events
    this.demuxer.on('segmentLoaded', (segmentIndex: number) => {
      this.log(`片段 #${segmentIndex} 已加载`, 'debug');

      // Update debug display if enabled
      if (this.options.debug?.enabled) {
        this.updateDebugDisplay();
      }
    });

    // Listen for errors
    this.demuxer.on('error', (error: Error) => {
      this.log(`播放器错误: ${error.message}`, 'error');
    });

    // Listen for debug messages
    this.demuxer.on('debug', (message: string) => {
      if (this.options.debug?.enabled) {
        this.log(message, 'debug');
      }
    });
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

  /**
   * 查找指定时间所属的时间范围
   */
  private findTimeRangeForTime(time: number): TimeRange | undefined {
    if (!this.options.timeRanges?.length) return undefined;

    // 遍历所有时间范围，查找包含指定时间的范围
    for (const range of this.options.timeRanges) {
      const rangeEnd = range.end;
      if (time >= range.start && time <= rangeEnd) {
        return range;
      }
    }

    // 如果未找到，使用最接近的范围
    let nearestRange: TimeRange | undefined;
    let minDistance = Number.MAX_VALUE;

    for (const range of this.options.timeRanges) {
      const startDistance = Math.abs(time - range.start);
      const endDistance = Math.abs(time - range.end);
      const minRangeDistance = Math.min(startDistance, endDistance);

      if (minRangeDistance < minDistance) {
        minDistance = minRangeDistance;
        nearestRange = range;
      }
    }

    // 如果存在最近的范围，记录日志
    if (nearestRange) {
      this.log(`时间 ${time}秒 不在任何时间范围内，使用最近的范围 [${nearestRange.start}-${nearestRange.end}]`, 'debug');
    }

    return nearestRange;
  }

  private async switchToTimeRange(range: TimeRange): Promise<void> {
    this.currentTimeRange = range;
    const relativeTime = this.currentMediaTime - range.mediaStart;
    await this.load(range.url);
    this.seek(range.start + relativeTime);
  }

  private originalToMediaTime(time: number): number {
    if (!this.options.timeRanges?.length) return time;

    // 查找时间所在的范围
    const range = this.findTimeRangeForTime(time);
    if (!range) return time; // 如果找不到范围，返回原始时间

    // 计算在原始视频中的偏移量
    const offsetInRange = time - range.start;

    // 转换为媒体时间
    return range.mediaStart + offsetInRange;
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
    this.container = container;

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

      // 添加详细进度信息标签
      const progressInfoLabel = document.createElement('div');
      progressInfoLabel.className = 'progress-info-label';
      progressInfoLabel.style.cssText = 'font-size: 12px; color: #666; margin-bottom: 4px; white-space: nowrap;';
      progressInfoLabel.textContent = '加载中...';
      progressContainer.appendChild(progressInfoLabel);
      this.progressInfoLabel = progressInfoLabel;

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
      this.progressInput.max = '100'; // 设置初始最大值为100
      this.progressInput.value = '0';
      this.progressInput.setAttribute('aria-label', '视频进度条');
      this.progressInput.oninput = this.handleSeek.bind(this);
      progressWrapper.appendChild(this.progressInput);

      progressContainer.appendChild(progressWrapper);

      this.timeDisplay = document.createElement('span');
      // 确保初始时间显示正确
      const initialTimeText = `${this.formatTime(0)} / ${this.formatTime(0)}`;
      console.log(`[HLSPlayer] 初始时间显示: ${initialTimeText}`);
      this.timeDisplay.textContent = initialTimeText;
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
    const duration = this.demuxer.getTotalDuration();

    if (this.options.timeRangeMode) {
      const { time, url } = this.calculateSeekTime(value);
      if (url && url !== this.currentTimeRange?.url) {
        this.load(url).then(() => this.seek(time));
      } else {
        this.seek(time);
      }
    } else {
      // 将百分比转换为实际时间
      const seekTime = (value / 100) * duration;
      console.log(`[HLSPlayer] 跳转: value=${value}%, duration=${duration}秒, seekTime=${seekTime}秒`);
      this.seek(seekTime);
    }
  }

  private updateProgress(): void {
    if (!this.options.showProgress) return;

    // 直接从视频元素获取当前时间，确保时间显示最新状态
    const currentTime = this.videoElement.currentTime;
    const rawDuration = this.demuxer.getTotalDuration();
    const demuxerTime = this.demuxer.getCurrentTime();

    // 记录当前时间和总时长，用于调试
    const now = Date.now();
    // 确保至少间隔 5 秒才输出一次日志
    if (now - HLSPlayer.lastProgressLogTime >= 5000) {
      console.log(`[HLSPlayer] 更新进度: currentTime=${currentTime.toFixed(2)}秒, totalDuration=${rawDuration.toFixed(2)}秒, demuxerTime=${demuxerTime.toFixed(2)}秒`);
      HLSPlayer.lastProgressLogTime = now;
    }

    // 更新进度信息标签
    if (this.progressInfoLabel) {
      // 获取缓冲区信息
      let bufferInfo = '';
      if (this.videoElement.buffered.length > 0) {
        for (let i = 0; i < this.videoElement.buffered.length; i++) {
          const start = this.videoElement.buffered.start(i);
          const end = this.videoElement.buffered.end(i);
          bufferInfo += `缓冲区 #${i}: ${start.toFixed(1)}-${end.toFixed(1)}秒 `;
        }
      } else {
        bufferInfo = '无缓冲区';
      }

      // 计算进度百分比
      const validDuration = isNaN(rawDuration) || rawDuration <= 0 || !isFinite(rawDuration) ? 100 : rawDuration;
      const progress = (currentTime / validDuration) * 100;

      // 更新标签内容
      this.progressInfoLabel.textContent = `当前时间: ${currentTime.toFixed(2)}秒 | ` +
        `Demuxer时间: ${demuxerTime.toFixed(2)}秒 | ` +
        `总时长: ${rawDuration.toFixed(2)}秒 | ` +
        `进度: ${progress.toFixed(1)}% | ` +
        `${bufferInfo} | ` +
        `播放状态: ${this.isPlaying() ? '播放中' : '已暂停'} | ` +
        `ReadyState: ${this.videoElement.readyState}`;
    }

    // Make sure we have valid duration to prevent NaN calculations
    const validDuration = isNaN(rawDuration) || rawDuration <= 0 || !isFinite(rawDuration) ? 100 : rawDuration;

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
          // 确保传递给 formatTime 的是有效数字
          const adjustedCurrentTime = Number(currentTime - totalStart);
          const adjustedTotalTime = Number(totalEnd - totalStart);
          console.log(`[HLSPlayer] 时间范围模式时间: adjustedCurrentTime=${adjustedCurrentTime}秒, adjustedTotalTime=${adjustedTotalTime}秒`);
          this.timeDisplay.textContent = `${this.formatTime(adjustedCurrentTime)} / ${this.formatTime(adjustedTotalTime)}`;
        }
      }

      // Update progress display for time ranges mode
      this.updateProgressDisplay();
    } else {
      if (this.progressInput) {
        // 计算进度百分比
        const progress = (currentTime / validDuration) * 100;
        this.progressInput.max = '100';
        this.progressInput.value = String(Math.min(100, Math.max(0, progress)));
      }

      if (this.timeDisplay) {
        // 使用视频元素的当前时间和 demuxer 提供的总时长，确保显示正确的时间
        // 确保传递给 formatTime 的是有效数字
        const numCurrentTime = Number(currentTime);
        const numValidDuration = Number(validDuration);
        console.log(`[HLSPlayer] 标准模式时间: numCurrentTime=${numCurrentTime}秒, numValidDuration=${numValidDuration}秒`);

        const formattedCurrentTime = this.formatTime(numCurrentTime);
        const formattedTotalTime = this.formatTime(numValidDuration);
        const timeDisplayText = `${formattedCurrentTime} / ${formattedTotalTime}`;

        console.log(`[HLSPlayer] 时间显示: ${timeDisplayText}`);
        this.timeDisplay.textContent = timeDisplayText;
      }

      // Update progress display for normal mode
      this.updateProgressDisplay();
    }

    if (this.playButton) {
      this.playButton.textContent = this.isPlaying() ? '暂停' : '播放';
    }

    // 更新调试显示
    if (this.options.debug?.enabled) {
      this.updateDebugDisplay();
    }
  }

  private formatTime(seconds: number): string {
    // 添加调试日志，查看传入的秒数
    console.log(`[HLSPlayer] formatTime 输入: ${seconds}秒, 类型: ${typeof seconds}, isNaN: ${isNaN(seconds)}, isFinite: ${isFinite(seconds)}`);

    // 确保 seconds 是有效的数字
    if (isNaN(seconds) || !isFinite(seconds) || seconds < 0) {
      console.log(`[HLSPlayer] formatTime 检测到无效时间，重置为0`);
      seconds = 0;
    }

    // 确保 seconds 是数字类型
    seconds = Number(seconds);

    const hours = Math.floor(seconds / 3600);
    const minutes = Math.floor((seconds % 3600) / 60);
    const secs = Math.floor(seconds % 60);

    let formattedTime;
    if (hours > 0) {
      formattedTime = `${hours.toString().padStart(2, '0')}:${minutes.toString().padStart(2, '0')}:${secs.toString().padStart(2, '0')}`;
    } else {
      formattedTime = `${minutes.toString().padStart(2, '0')}:${secs.toString().padStart(2, '0')}`;
    }

    // 添加调试日志，查看格式化后的时间
    console.log(`[HLSPlayer] formatTime 输出: ${formattedTime}, 原始秒数: ${seconds}秒`);

    return formattedTime;
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
    this.log(`加载HLS地址: ${url}`, 'info');

    try {
      // 销毁之前的实例(如果有)
      if (this.demuxer) {
        this.demuxer.destroy();

        // 重新创建demuxer实例
        this.demuxer = new HLSv7Demuxer(this.videoElement, {
          logLevel: (this.options.debug?.enabled ? 'debug' : 'info'),
          slidingWindow: {
            enabled: true,
            forward: 2,
            backward: 1
          }
        });

        // 重新设置事件监听器
        this.setupDemuxerListeners();
      }

      // 显示加载指示器
      if (this.loadingIndicator) {
        this.loadingIndicator.style.display = 'block';
      }

      // 初始化解复用器
      await this.demuxer.init(url);

      // 初始化进度条
      if (this.options.showProgress) {
        this.updateProgressDisplay();
      }

      // 更新调试UI(如果启用)
      if (this.options.debug?.enabled) {
        this.updateDebugDisplay();
      }

      // 隐藏加载指示器
      if (this.loadingIndicator) {
        this.loadingIndicator.style.display = 'none';
      }
    } catch (error) {
      console.error('加载媒体失败:', error);

      // 隐藏加载指示器
      if (this.loadingIndicator) {
        this.loadingIndicator.style.display = 'none';
      }

      throw error;
    }
  }

  public async play(): Promise<void> {
    this.log(`开始播放`, 'info');
    try {
      // 让demuxer处理播放逻辑
      await this.demuxer.play();

      // 开始进度更新
      this.startProgressUpdate();

      // 更新播放按钮状态
      if (this.playButton) {
        this.playButton.textContent = '暂停';
      }
    } catch (error) {
      console.error(`播放失败:`, error);

      // 尝试直接播放
      try {
        await this.videoElement.play();
        this.startProgressUpdate();
        if (this.playButton) {
          this.playButton.textContent = '暂停';
        }
      } catch (innerError) {
        console.error(`直接播放也失败:`, innerError);
      }
    }
  }

  public pause(): void {
    // 使用demuxer的pause方法
    this.demuxer.pause();
    this.stopProgressUpdate();

    // 更新播放按钮状态
    if (this.playButton) {
      this.playButton.textContent = '播放';
    }

    this.updateProgress();
  }

  /**
   * 跳转到指定时间
   */
  public seek(time: number): void {
    if (!this.demuxer) return;

    const startTime = performance.now();
    this.log(`尝试跳转: ${this.videoElement.currentTime.toFixed(2)}s → ${time.toFixed(2)}s`, 'info');

    // 如果使用时间范围模式，则需要转换时间
    if (this.options.timeRangeMode && this.options.timeRanges?.length) {
      // 查找时间所在的范围
      const range = this.findTimeRangeForTime(time);

      if (range && this.currentTimeRange !== range) {
        // 如果跳转到了不同的时间范围，需要切换范围
        this.switchToTimeRange(range);
        return;
      }

      // 转换为媒体时间
      const mediaTime = this.originalToMediaTime(time);

      // 使用demuxer进行跳转
      this.demuxer.seek(mediaTime);
    } else {
      // 普通模式，直接跳转
      this.demuxer.seek(time);
    }

    // 强制更新进度显示
    this.forceUpdateProgress();

    const seekDuration = performance.now() - startTime;
    this.log(`跳转完成，耗时 ${seekDuration.toFixed(2)}ms`, 'debug');
  }

  /**
   * 获取当前播放时间
   */
  public getCurrentTime(): number {
    if (!this.demuxer) return 0;

    const time = this.demuxer.getCurrentTime();

    // 如果使用时间范围模式，则需要转换时间
    if (this.options.timeRangeMode && this.options.timeRanges?.length) {
      return this.mediaToOriginalTime(time);
    }

    return time;
  }

  /**
   * 获取总时长
   */
  public getDuration(): number {
    if (!this.demuxer) return 0;

    // 如果使用时间范围模式，返回所有范围的总时长
    if (this.options.timeRangeMode && this.options.timeRanges?.length) {
      return this.options.timeRanges.reduce((total, range) => total + (range.end - range.start), 0);
    }

    return this.demuxer.getTotalDuration();
  }

  /**
   * 设置播放速率
   */
  public setPlaybackRate(rate: number): void {
    if (!this.demuxer) return;

    this.demuxer.setPlaybackRate(rate);
    this.videoElement.playbackRate = rate;

    // 更新播放速率按钮状态
    this.rateButtons.forEach(button => {
      button.classList.toggle('active', parseFloat(button.dataset.rate || '1') === rate);
    });
  }

  /**
   * 销毁播放器实例
   */
  public destroy(): void {
    // 停止进度更新
    this.stopProgressUpdate();

    // 销毁demuxer
    if (this.demuxer) {
      this.demuxer.destroy();
    }

    // 移除事件监听器
    this.videoElement.removeEventListener('timeupdate', this.handleTimeUpdate.bind(this));

    // 清理UI元素
    if (this.container && this.container.parentNode) {
      this.container.parentNode.removeChild(this.container);
    }
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
    if (!this.progressCanvas) return;
    const ctx = this.progressCanvas.getContext('2d');
    if (!ctx) return;

    const canvas = this.progressCanvas;
    // 设置 canvas 尺寸
    canvas.width = canvas.offsetWidth * window.devicePixelRatio;
    canvas.height = canvas.offsetHeight * window.devicePixelRatio;

    // 清除画布
    ctx.clearRect(0, 0, canvas.width, canvas.height);

    if (this.options.timeRangeMode && this.options.timeRanges?.length) {
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
    } else {
      // 直接从视频元素获取当前时间，确保时间轴显示最新状态
      const currentTime = this.videoElement.currentTime;
      // 从 demuxer 获取总时长
      const duration = this.demuxer.getTotalDuration();
      const validDuration = isNaN(duration) || duration <= 0 || !isFinite(duration) ? 100 : duration;

      // 静态属性，用于记录上次时间轴渲染日志输出时间
      if (!HLSPlayer.lastTimelineLogTime) {
        HLSPlayer.lastTimelineLogTime = 0;
      }

      const now = Date.now();
      // 确保至少间隔 10 秒才输出一次日志
      if (now - HLSPlayer.lastTimelineLogTime >= 10000) {
        console.log(`[HLSPlayer] 渲染时间轴: currentTime=${currentTime.toFixed(2)}秒, duration=${validDuration.toFixed(2)}秒, videoCurrentTime=${this.videoElement.currentTime.toFixed(2)}秒`);
        HLSPlayer.lastTimelineLogTime = now;
      }

      // 绘制背景
      ctx.fillStyle = 'rgba(0, 0, 0, 0.2)';
      ctx.fillRect(0, 0, canvas.width, canvas.height);

      // 绘制已缓冲区域
      if (this.videoElement.buffered.length > 0) {
        ctx.fillStyle = 'rgba(0, 160, 255, 0.3)';
        for (let i = 0; i < this.videoElement.buffered.length; i++) {
          const start = this.videoElement.buffered.start(i);
          const end = this.videoElement.buffered.end(i);
          const startX = (start / validDuration) * canvas.width;
          const width = ((end - start) / validDuration) * canvas.width;
          ctx.fillRect(startX, 0, width, canvas.height);
        }
      }

      // 绘制已播放区域
      if (currentTime > 0) {
        ctx.fillStyle = 'rgba(24, 160, 88, 0.5)';
        const playedWidth = (currentTime / validDuration) * canvas.width;
        ctx.fillRect(0, 0, playedWidth, canvas.height);

        // 绘制当前播放位置指示器
        ctx.fillStyle = '#18a058';
        ctx.fillRect(playedWidth - 1, 0, 2, canvas.height);
      }
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

  // 添加一个方法，强制更新时间轴
  public forceUpdateProgress(): void {
    // 立即更新进度显示
    this.updateProgress();

    // 确保时间轴渲染使用最新的时间值
    if (this.videoElement) {
      // 触发 timeupdate 事件，确保 UI 更新
      this.videoElement.dispatchEvent(new Event('timeupdate'));

      // 额外更新进度信息标签，添加强制更新标记
      if (this.progressInfoLabel) {
        const currentText = this.progressInfoLabel.textContent || '';
        this.progressInfoLabel.textContent = `${currentText} | [强制更新]`;

        // 短暂改变标签颜色以突出显示更新
        const originalColor = this.progressInfoLabel.style.color;
        this.progressInfoLabel.style.color = '#ff5500';
        setTimeout(() => {
          if (this.progressInfoLabel) {
            this.progressInfoLabel.style.color = originalColor;
          }
        }, 1000);
      }

      // 直接更新时间显示，确保显示正确
      if (this.timeDisplay) {
        const currentTime = Number(this.videoElement.currentTime);
        const duration = Number(this.demuxer.getTotalDuration());
        const validDuration = isNaN(duration) || duration <= 0 || !isFinite(duration) ? 100 : duration;

        const formattedCurrentTime = this.formatTime(currentTime);
        const formattedTotalTime = this.formatTime(validDuration);
        const timeDisplayText = `${formattedCurrentTime} / ${formattedTotalTime}`;

        console.log(`[HLSPlayer] 强制更新时间显示: ${timeDisplayText}`);
        this.timeDisplay.textContent = timeDisplayText;
      }
    }

    // 记录强制更新日志
    console.log(`[HLSPlayer] 强制更新进度: currentTime=${this.videoElement.currentTime.toFixed(2)}秒, demuxerTime=${this.demuxer.getCurrentTime().toFixed(2)}秒`);
  }

  /**
   * 记录日志信息
   */
  private log(message: string, level: 'info' | 'debug' | 'error' = 'info'): void {
    const prefix = '[HLSPlayer]';
    if (level === 'error') {
      console.error(`${prefix} ${message}`);
    } else if (level === 'info') {
      console.log(`${prefix} ${message}`);
    } else if (level === 'debug' && this.options.debug?.enabled) {
      console.debug(`${prefix} ${message}`);
    }
  }

  /**
   * 配置滑动窗口参数
   */
  public setSlidingWindowConfig(config: Partial<SlidingWindowConfig>): void {
    if (!this.demuxer) return;

    this.demuxer.setSlidingWindowConfig(config);
    this.log(`已更新滑动窗口配置: ${JSON.stringify(config)}`, 'debug');
  }

  /**
   * 获取当前滑动窗口配置
   */
  public getSlidingWindowConfig(): SlidingWindowConfig | undefined {
    if (!this.demuxer) return undefined;

    return this.demuxer.getSlidingWindowConfig();
  }

  /**
   * 获取视频片段总数
   */
  public getTotalSegments(): number {
    if (!this.demuxer) return 0;

    return this.demuxer.getTotalSegments();
  }

  /**
   * 检查播放器是否正在播放
   */
  public isPlaying(): boolean {
    return !this.videoElement.paused;
  }
}