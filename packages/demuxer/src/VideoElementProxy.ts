import { EventBus, Events } from './EventBus';

/**
 * VideoElementProxy - 视频元素代理
 * 负责封装HTMLVideoElement，处理视频相关事件，并通过事件总线转发
 */
export class VideoElementProxy {
  private eventBus: EventBus;
  private videoElement: HTMLVideoElement;
  private initialized: boolean = false;
  private currentSrc: string | null = null;
  
  /**
   * 构造函数
   * @param eventBus 事件总线
   * @param videoElement HTML视频元素
   */
  constructor(eventBus: EventBus, videoElement: HTMLVideoElement) {
    this.eventBus = eventBus;
    this.videoElement = videoElement;
    this.setupEventListeners();
  }
  
  /**
   * 设置视频元素的事件监听
   */
  private setupEventListeners(): void {
    // 时间更新事件
    this.videoElement.addEventListener('timeupdate', this.handleTimeUpdate);
    
    // 播放控制事件
    this.videoElement.addEventListener('play', this.handlePlay);
    this.videoElement.addEventListener('pause', this.handlePause);
    
    // 跳转相关事件
    this.videoElement.addEventListener('seeking', this.handleSeeking);
    this.videoElement.addEventListener('seeked', this.handleSeeked);
    
    // 加载和错误事件
    this.videoElement.addEventListener('loadedmetadata', this.handleLoadedMetadata);
    this.videoElement.addEventListener('error', this.handleError);
    
    // 缓冲事件
    this.videoElement.addEventListener('progress', this.handleProgress);
    this.videoElement.addEventListener('waiting', this.handleWaiting);
    this.videoElement.addEventListener('canplay', this.handleCanPlay);
    
    // 响应currentTime:request事件
    this.eventBus.on(Events.CURRENT_TIME_REQUEST, this.handleCurrentTimeRequest);
    
    // 监听MediaSource URL更新事件
    this.eventBus.on('video:updateSource', this.handleSourceUpdate);
  }
  
  /**
   * 移除视频元素的事件监听
   */
  public removeEventListeners(): void {
    this.videoElement.removeEventListener('timeupdate', this.handleTimeUpdate);
    this.videoElement.removeEventListener('play', this.handlePlay);
    this.videoElement.removeEventListener('pause', this.handlePause);
    this.videoElement.removeEventListener('seeking', this.handleSeeking);
    this.videoElement.removeEventListener('seeked', this.handleSeeked);
    this.videoElement.removeEventListener('loadedmetadata', this.handleLoadedMetadata);
    this.videoElement.removeEventListener('error', this.handleError);
    this.videoElement.removeEventListener('progress', this.handleProgress);
    this.videoElement.removeEventListener('waiting', this.handleWaiting);
    this.videoElement.removeEventListener('canplay', this.handleCanPlay);
    
    this.eventBus.off(Events.CURRENT_TIME_REQUEST, this.handleCurrentTimeRequest);
    this.eventBus.off('video:updateSource', this.handleSourceUpdate);
  }
  
  /**
   * 处理MediaSource URL更新
   */
  private handleSourceUpdate = (url: string): void => {
    this.setSource(url);
  }
  
  /**
   * 处理视频时间更新
   */
  private handleTimeUpdate = (): void => {
    this.eventBus.emit(Events.VIDEO_TIME_UPDATE, this.videoElement.currentTime);
  }
  
  /**
   * 处理视频播放事件
   */
  private handlePlay = (): void => {
    this.eventBus.emit(Events.VIDEO_PLAY);
  }
  
  /**
   * 处理视频暂停事件
   */
  private handlePause = (): void => {
    this.eventBus.emit(Events.VIDEO_PAUSE);
  }
  
  /**
   * 处理视频seeking事件
   */
  private handleSeeking = (): void => {
    this.eventBus.emit(Events.VIDEO_SEEKING, this.videoElement.currentTime);
  }
  
  /**
   * 处理视频seeked事件
   */
  private handleSeeked = (): void => {
    this.eventBus.emit(Events.VIDEO_SEEKED, this.videoElement.currentTime);
  }
  
  /**
   * 处理视频元数据加载事件
   */
  private handleLoadedMetadata = (): void => {
    this.initialized = true;
    this.eventBus.emit('video:loadedMetadata', {
      duration: this.videoElement.duration,
      videoWidth: this.videoElement.videoWidth,
      videoHeight: this.videoElement.videoHeight
    });
  }
  
  /**
   * 处理视频加载进度事件
   */
  private handleProgress = (): void => {
    const buffered = this.videoElement.buffered;
    const bufferRanges = [];
    
    for (let i = 0; i < buffered.length; i++) {
      bufferRanges.push({
        start: buffered.start(i),
        end: buffered.end(i)
      });
    }
    
    this.eventBus.emit('video:buffered', bufferRanges);
  }
  
  /**
   * 处理视频等待缓冲事件
   */
  private handleWaiting = (): void => {
    this.eventBus.emit('video:waiting');
  }
  
  /**
   * 处理视频可以播放事件
   */
  private handleCanPlay = (): void => {
    this.eventBus.emit('video:canPlay');
  }
  
  /**
   * 处理视频错误事件
   */
  private handleError = (e: Event): void => {
    const videoElement = e.target as HTMLVideoElement;
    const error = videoElement.error;
    
    this.log(`视频元素错误: ${error}`, 'error');
    this.eventBus.emit(Events.VIDEO_ERROR, error);
    
    // 尝试重新应用视频源以恢复播放
    if (this.currentSrc) {
      setTimeout(() => {
        this.log('Attempting to reapply video source after error', 'info');
        if (this.currentSrc) {
          this.setSource(this.currentSrc);
        }
      }, 1000);
    }
  }
  
  /**
   * 处理获取当前时间的请求
   */
  private handleCurrentTimeRequest = (): number => {
    return this.videoElement.currentTime;
  }
  
  /**
   * 设置视频源
   * @param url 视频URL
   */
  public setSource(url: string): void {
    this.log(`Setting video source: ${url}`, 'debug');
    
    // 保存当前URL以便可能的错误恢复
    this.currentSrc = url;
    
    // 如果当前视频处于播放状态，记录此状态以便重新应用
    const wasPlaying = !this.videoElement.paused;
    
    // 设置新的源
    this.videoElement.src = url;
    
    // 加载新的源以触发事件
    this.videoElement.load();
    
    // 如果之前在播放，则尝试恢复播放
    if (wasPlaying) {
      // 使用Promise捕获可能的自动播放错误
      const playPromise = this.videoElement.play();
      if (playPromise !== undefined) {
        playPromise.catch(error => {
          this.log(`Auto-resume play failed: ${error}`, 'warning');
        });
      }
    }
  }
  
  /**
   * 获取视频元素
   */
  public getVideoElement(): HTMLVideoElement {
    return this.videoElement;
  }
  
  /**
   * 强制重新加载视频
   */
  public reload(): void {
    if (this.currentSrc) {
      this.setSource(this.currentSrc);
    }
  }
  
  /**
   * 销毁代理，清理资源
   */
  public destroy(): void {
    this.removeEventListeners();
    this.videoElement.src = '';
    this.currentSrc = null;
  }
  
  /**
   * 日志输出
   */
  private log(message: string, level: 'debug' | 'info' | 'warning' | 'error' = 'debug'): void {
    this.eventBus.emit(Events.LOG, message, level);
  }
} 