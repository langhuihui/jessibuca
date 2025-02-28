import { EventBus, Events } from "./EventBus";
import { VirtualTimeline, MediaSegment, PlaylistInfo, BufferRange as VirtualBufferRange } from './VirtualTimeline';
import { SegmentLoader, SlidingWindowConfig, BufferRange as SegmentLoaderBufferRange } from './SegmentLoader';
import { VideoElementProxy } from "./VideoElementProxy";
import { MediaSourceManager } from "./MediaSourceManager";

// 接口定义
interface Mp4Segment {
  url: string;
  duration: number;
  time: Date;
}

interface BufferRange {
  start: number;
  end: number;
}

interface DemuxerOptions {
  codec?: string;
  maxBufferLength?: number;
  logLevel?: LogLevel;
  slidingWindow?: Partial<SlidingWindowConfig>;
}

type LogLevel = 'debug' | 'info' | 'error' | 'warning';

type HLSv7DemuxerEvents = {
  playlistUpdate: (playlist: Mp4Segment[]) => void;
  segmentLoaded: (segmentIndex: number) => void;
  error: (error: Error) => void;
  bufferUpdate: (ranges: BufferRange[]) => void;
  debug: (message: string) => void;
  demuxerInitialized: () => void;
};

/**
 * HLSv7Demuxer - HLS解码器
 * 
 * 作为整个系统的协调者，负责初始化和管理各个组件
 * 提供统一的公共API用于控制播放、加载和管理HLS流
 * 通过事件总线进行组件间通信，减少直接依赖
 */
export class HLSv7Demuxer {
  // 核心组件
  private eventBus: EventBus;
  private videoElementProxy: VideoElementProxy;
  private mediaSourceManager: MediaSourceManager;
  private virtualTimeline: VirtualTimeline;
  private segmentLoader: SegmentLoader;
  
  // 状态标志
  private isDestroyed: boolean = false;
  private totalDuration: number = 0;
  private currentSegmentIndex: number = 0;
  private isInitialized: boolean = false;
  
  // 卡顿检测
  private lastTimeUpdateValue: number = 0;
  private lastTimeUpdateTimestamp: number = 0;
  private stuckDuration: number = 0;
  private stuckDetectionThreshold: number = 2000; // 2秒阈值
  
  // 配置参数
  private codec: string;
  private maxBufferLength: number;
  private logLevel: LogLevel;
  
  // 事件回调
  private eventHandlers?: {
    error: (error: Error) => void;
    playlistUpdate: (segments: Mp4Segment[]) => void;
    bufferUpdate: (ranges: BufferRange[]) => void;
    debug: (message: string) => void;
    demuxerInitialized: () => void;
    segmentLoaded: (segmentIndex: number) => void;
  };
  
  /**
   * 构造函数
   * @param videoElement 视频元素
   * @param options 配置选项
   */
  constructor(videoElement: HTMLVideoElement, options: DemuxerOptions = {}) {
    // 初始化事件总线
    this.eventBus = new EventBus();
    
    // 初始化配置
    this.codec = options.codec || 'video/mp4; codecs="avc1.640028"';
    this.maxBufferLength = options.maxBufferLength || 30;
    this.logLevel = options.logLevel || 'debug';
    
    // 创建各个组件
    this.videoElementProxy = new VideoElementProxy(this.eventBus, videoElement);
    this.mediaSourceManager = new MediaSourceManager(this.eventBus);
    this.virtualTimeline = new VirtualTimeline(this.eventBus);
    this.segmentLoader = new SegmentLoader(this.eventBus);
    
    // 设置滑动窗口配置
    if (options.slidingWindow) {
      this.segmentLoader.setSlidingWindowConfig(options.slidingWindow);
    }
    
    // 设置事件监听
    this.setupEventListeners();
    
    this.log('HLSv7Demuxer initialized', 'info');
  }
  
  /**
   * 设置事件监听
   */
  private setupEventListeners(): void {
    // 监听错误事件
    this.eventBus.on(Events.ERROR, this.handleError);
    
    // 监听日志事件
    this.eventBus.on(Events.LOG, this.handleLog);
    
    // 监听MediaSource事件
    this.eventBus.on(Events.MEDIA_SOURCE_OPEN, this.handleMediaSourceOpen);
    
    // 监听缓冲区更新事件
    this.eventBus.on(Events.BUFFER_UPDATE, this.handleBufferUpdate);
    
    // 监听片段事件
    this.eventBus.on('segmentLoaded', this.handleSegmentLoaded);
    this.eventBus.on('segmentLoadError', this.handleSegmentLoadError);
    
    // 监听附加缓冲区请求
    this.eventBus.on('appendBuffer', this.handleAppendBuffer);
    this.eventBus.on('appendInitSegment', this.handleAppendInitSegment);
    this.eventBus.on('requestClearBuffer', this.handleClearBuffer);
    this.eventBus.on('requestRemoveSegment', this.handleRemoveSegment);
    
    // 响应编解码器请求
    this.eventBus.on(Events.CODEC_REQUEST, () => this.codec);
    
    // 监听视频错误事件
    this.eventBus.on(Events.VIDEO_ERROR, this.handleVideoError);
  }
  
  /**
   * 移除事件监听
   */
  private removeEventListeners(): void {
    this.eventBus.off(Events.MEDIA_SOURCE_OPEN, this.handleMediaSourceOpen);
    this.eventBus.off(Events.ERROR, this.handleError);
    this.eventBus.off(Events.LOG, this.handleLog);
    this.eventBus.off(Events.BUFFER_UPDATE, this.handleBufferUpdate);
    this.eventBus.off(Events.SEGMENT_LOADED, this.handleSegmentLoaded);
    this.eventBus.off(Events.SEGMENT_LOAD_ERROR, this.handleSegmentLoadError);
    this.eventBus.off(Events.APPEND_BUFFER, this.handleAppendBuffer);
    this.eventBus.off(Events.APPEND_INIT_SEGMENT, this.handleAppendInitSegment);
    this.eventBus.off('requestClearBuffer', this.handleClearBuffer);
    this.eventBus.off('requestRemoveSegment', this.handleRemoveSegment);
    this.eventBus.off(Events.CODEC_REQUEST, this.handleCodecRequest);
    this.eventBus.off(Events.VIDEO_ERROR, this.handleVideoError);
  }
  
  /**
   * 处理错误事件
   */
  private handleError = (message: string, error?: any): void => {
    this.log(`Error: ${message}${error ? ` - ${error}` : ''}`, 'error');
    
    // 检查是否是MediaSource错误，如果是，尝试恢复
    if (error && (message.includes('MediaSource') || message.includes('sourceopen'))) {
      this.log('Attempting to recover from MediaSource error...', 'info');
      
      // 尝试强制打开MediaSource
      this.mediaSourceManager.forceSourceOpen();
    }
    
    // 触发错误事件回调
    if (this.eventHandlers?.error) {
      this.eventHandlers.error(new Error(message));
    }
  }
  
  /**
   * 处理日志事件
   */
  private handleLog = (message: string, level: LogLevel): void => {
    if (this.shouldLog(level)) {
      this.log(message, level);
    }
  }
  
  /**
   * 处理MediaSource打开事件
   */
  private handleMediaSourceOpen = (mediaSource: MediaSource): void => {
    this.log('MediaSource opened successfully', 'info');
    
    // 获取初始化片段URL
    const initSegmentUrl = this.extractInitSegmentUrl(this.getFirstSegment()?.url || '');
    
    // 加载初始化片段
    if (initSegmentUrl) {
      this.setInitSegmentUrl(initSegmentUrl);
    }
    
    // 触发初始化完成事件
    if (this.eventHandlers?.demuxerInitialized) {
      this.eventHandlers.demuxerInitialized();
    }
  }
  
  /**
   * 处理缓冲区更新事件
   */
  private handleBufferUpdate = (ranges: BufferRange[]): void => {
    // 触发缓冲区更新事件
    if (this.eventHandlers?.bufferUpdate) {
      this.eventHandlers.bufferUpdate(ranges);
    }
  }
  
  /**
   * 处理片段加载完成事件
   */
  private handleSegmentLoaded = (segmentIndex: number): void => {
    // 触发片段加载完成事件
    if (this.eventHandlers?.segmentLoaded) {
      this.eventHandlers.segmentLoaded(segmentIndex);
    }
  }
  
  /**
   * 处理片段加载错误事件
   */
  private handleSegmentLoadError = (segmentIndex: number, error: any): void => {
    this.log(`Segment #${segmentIndex} load error: ${error}`, 'error');
    // 触发错误事件
    if (this.eventHandlers?.error) {
      this.eventHandlers.error(new Error(`Segment #${segmentIndex} load error: ${error}`));
    }
  }
  
  /**
   * 处理添加缓冲区请求
   */
  private handleAppendBuffer = (data: ArrayBuffer, segmentIndex: number): void => {
    const sourceBuffer = this.mediaSourceManager.getSourceBuffer();
    if (!sourceBuffer) {
      this.log('Cannot append buffer, SourceBuffer not available', 'error');
      return;
    }
    
    try {
      sourceBuffer.appendBuffer(data);
    } catch (error) {
      this.log(`Error appending buffer for segment #${segmentIndex}: ${error}`, 'error');
      this.handleError('Buffer append error', error);
    }
  }
  
  /**
   * 处理添加初始化片段请求
   */
  private handleAppendInitSegment = (data: ArrayBuffer): void => {
    const sourceBuffer = this.mediaSourceManager.getSourceBuffer();
    if (!sourceBuffer) {
      this.log('Cannot append init segment, SourceBuffer not available', 'error');
      return;
    }
    
    try {
      sourceBuffer.appendBuffer(data);
    } catch (error) {
      this.log(`Error appending init segment: ${error}`, 'error');
      this.handleError('Init segment append error', error);
    }
  }
  
  /**
   * 处理清除缓冲区请求
   */
  private handleClearBuffer = (): void => {
    const sourceBuffer = this.mediaSourceManager.getSourceBuffer();
    if (!sourceBuffer || !this.mediaSourceManager.isMediaSourceReady()) {
      this.log('Cannot clear buffer, SourceBuffer not available or MediaSource not ready', 'warning');
      return;
    }
    
    try {
      // 清除整个缓冲区
      sourceBuffer.remove(0, Infinity);
    } catch (error) {
      this.log(`Error clearing buffer: ${error}`, 'error');
    }
  }
  
  /**
   * 处理移除片段请求
   */
  private handleRemoveSegment = (segmentIndex: number): void => {
    const sourceBuffer = this.mediaSourceManager.getSourceBuffer();
    if (!sourceBuffer || !this.mediaSourceManager.isMediaSourceReady()) {
      this.log(`Cannot remove segment #${segmentIndex}, SourceBuffer not available or MediaSource not ready`, 'warning');
      return;
    }
    
    const segment = this.findSegmentByIndex(segmentIndex);
    if (!segment) {
      this.log(`Cannot find segment #${segmentIndex} to remove`, 'warning');
      return;
    }
    
    try {
      // 移除指定范围的缓冲区
      sourceBuffer.remove(segment.virtualStartTime, segment.virtualEndTime);
    } catch (error) {
      this.log(`Error removing segment #${segmentIndex}: ${error}`, 'error');
    }
  }
  
  /**
   * 初始化解码器
   * @param url M3U8播放列表URL
   */
  public async init(url: string): Promise<void> {
    this.checkDestroyed();
    
    try {
      this.log(`Initializing with M3U8 url: ${url}`, 'info');
      
      // 加载播放列表
      const playlistInfo = await this.fetchPlaylist(url);
      
      // 设置片段到虚拟时间轴
      this.setPlaylist(playlistInfo);
      this.totalDuration = this.virtualTimeline.getTotalDuration();
      
      // 创建MediaSource并设置到视频元素
      const mediaSourceUrl = this.mediaSourceManager.createMediaSource();
      
      // 触发播放列表更新事件
      if (this.eventHandlers?.playlistUpdate) {
        this.eventHandlers.playlistUpdate(playlistInfo.segments.map(segment => ({
          url: segment.url,
          duration: segment.duration,
          time: segment.physicalTime || new Date()
        })));
      }
      
      // 设置初始化完成状态
      this.isInitialized = true;
    } catch (error) {
      this.log(`Failed to initialize: ${error}`, 'error');
      if (this.eventHandlers?.error) {
        this.eventHandlers.error(new Error(`Failed to initialize: ${error}`));
      }
      throw error;
    }
  }
  
  /**
   * 获取当前时间
   * @returns 当前虚拟时间
   */
  public getCurrentTime(): number {
    this.checkDestroyed();
    
    // 首先尝试从VirtualTimeline获取时间
    const virtualTime = this.eventBus.request(Events.CURRENT_TIME_REQUEST);
    
    // 如果VirtualTimeline返回0，则尝试从视频元素获取时间
    if (virtualTime === 0) {
      const videoTime = this.eventBus.request('video:currentTime');
      if (videoTime > 0) {
        this.log(`Using video element time (${videoTime}s) as fallback`, 'debug');
        return videoTime;
      }
    }
    
    return virtualTime;
  }
  
  /**
   * 获取总时长
   * @returns 总时长
   */
  public getTotalDuration(): number {
    this.checkDestroyed();
    return this.totalDuration;
  }
  
  /**
   * 获取播放进度
   * @returns 播放进度(0-1)
   */
  public getProgress(): number {
    this.checkDestroyed();
    
    const currentTime = this.getCurrentTime();
    if (this.totalDuration <= 0) {
      return 0;
    }
    
    return Math.max(0, Math.min(1, currentTime / this.totalDuration));
  }
  
  /**
   * 开始播放
   */
  public async play(): Promise<void> {
    this.checkDestroyed();
    
    try {
      // 请求视频元素播放
      this.eventBus.emit('video:play');
      
      // 启动VirtualTimeline播放
      this.virtualTimeline.play();
      
      this.log('Playback started', 'info');
    } catch (error) {
      this.log(`Failed to start playback: ${error}`, 'error');
      throw error;
    }
  }
  
  /**
   * 暂停播放
   */
  public pause(): void {
    this.checkDestroyed();
    
    // 请求视频元素暂停
    this.eventBus.emit('video:pause');
    
    // 暂停VirtualTimeline
    this.virtualTimeline.pause();
    
    this.log('Playback paused', 'info');
  }
  
  /**
   * 跳转到指定时间
   * @param time 目标时间(秒)
   */
  public async seek(time: number): Promise<void> {
    this.checkDestroyed();
    
    try {
      // 确保时间在有效范围内
      time = Math.max(0, Math.min(this.totalDuration, time));
      
      // 请求VirtualTimeline跳转
      this.virtualTimeline.seek(time);
      
      // 请求视频元素跳转
      this.eventBus.emit('video:seek', time);
      
      this.log(`Seeked to ${time}s`, 'info');
    } catch (error) {
      this.log(`Failed to seek: ${error}`, 'error');
      throw error;
    }
  }
  
  /**
   * 设置播放速率
   * @param rate 播放速率
   */
  public setPlaybackRate(rate: number): void {
    this.checkDestroyed();
    
    // 请求视频元素设置播放速率
    this.eventBus.emit('video:setPlaybackRate', rate);
    
    // 设置VirtualTimeline播放速率
    this.virtualTimeline.setPlaybackRate(rate);
    
    this.log(`Playback rate set to ${rate}x`, 'info');
  }
  
  /**
   * 清除缓冲区
   */
  public async clearBuffer(): Promise<void> {
    this.checkDestroyed();
    
    try {
      // 请求SegmentLoader清除所有片段
      this.segmentLoader.clearAllSegments();
      
      this.log('Buffer cleared', 'info');
    } catch (error) {
      this.log(`Failed to clear buffer: ${error}`, 'error');
      throw error;
    }
  }
  
  /**
   * 获取片段总数
   * @returns 片段总数
   */
  public getTotalSegments(): number {
    this.checkDestroyed();
    
    // 向SegmentLoader请求片段总数
    const segments = this.findAllSegments();
    return segments.length;
  }
  
  /**
   * 获取所有片段
   * @returns 所有片段
   */
  private findAllSegments(): MediaSegment[] {
    // 从事件总线请求片段
    const segments = this.eventBus.request('segments:getAll');
    return segments || [];
  }
  
  /**
   * 根据索引查找片段
   * @param index 片段索引
   * @returns 片段或null
   */
  private findSegmentByIndex(index: number): MediaSegment | null {
    const segments = this.findAllSegments();
    return segments[index] || null;
  }
  
  /**
   * 设置滑动窗口配置
   * @param config 滑动窗口配置
   */
  public setSlidingWindowConfig(config: Partial<SlidingWindowConfig>): void {
    this.checkDestroyed();
    this.segmentLoader.setSlidingWindowConfig(config);
  }
  
  /**
   * 获取滑动窗口配置
   * @returns 滑动窗口配置
   */
  public getSlidingWindowConfig(): SlidingWindowConfig {
    this.checkDestroyed();
    return this.segmentLoader.getSlidingWindowConfig();
  }
  
  /**
   * 销毁解码器
   */
  public destroy(): void {
    if (this.isDestroyed) {
      return;
    }
    
    this.log('Destroying HLSv7Demuxer', 'info');
    
    // 移除事件监听
    this.removeEventListeners();
    
    // 销毁各个组件
    this.videoElementProxy.destroy();
    this.mediaSourceManager.destroy();
    this.virtualTimeline.destroy();
    this.segmentLoader.destroy();
    
    // 清理事件总线
    this.eventBus.clear();
    
    // 标记为已销毁
    this.isDestroyed = true;
  }
  
  /**
   * 获取MediaSource对象
   * @returns MediaSource对象
   */
  public getMediaSource(): MediaSource | null {
    this.checkDestroyed();
    return this.mediaSourceManager.getMediaSource();
  }
  
  /**
   * 从URL提取初始化片段URL
   * @param url 播放列表URL
   * @returns 初始化片段URL
   */
  private extractInitSegmentUrl(url: string): string | null {
    // 从URL中提取目录
    const lastSlashIndex = url.lastIndexOf('/');
    if (lastSlashIndex === -1) {
      return null;
    }
    
    const baseUrl = url.substring(0, lastSlashIndex + 1);
    return `${baseUrl}init.mp4`;
  }
  
  /**
   * 获取M3U8播放列表
   * @param url 播放列表URL
   * @returns 播放列表信息
   */
  private async fetchPlaylist(url: string): Promise<PlaylistInfo> {
    try {
      const response = await fetch(url);
      if (!response.ok) {
        throw new Error(`HTTP error! status: ${response.status}`);
      }
      
      const m3u8Content = await response.text();
      return this.parseM3U8Playlist(url, m3u8Content);
    } catch (error) {
      this.log(`Failed to fetch playlist: ${error}`, 'error');
      throw error;
    }
  }
  
  /**
   * 解析M3U8播放列表
   * @param url 播放列表URL
   * @param content 播放列表内容
   * @returns 播放列表信息
   */
  private parseM3U8Playlist(url: string, content: string): PlaylistInfo {
    // 解析基础URL
    const baseUrl = this.getBaseUrl(url);
    
    // 使用VirtualTimeline解析M3U8
    return this.virtualTimeline.createPlaylistFromM3U8(content, baseUrl);
  }
  
  /**
   * 获取基础URL
   * @param url 完整URL
   * @returns 基础URL
   */
  private getBaseUrl(url: string): string {
    const lastSlashIndex = url.lastIndexOf('/');
    if (lastSlashIndex === -1) {
      return url;
    }
    return url.substring(0, lastSlashIndex + 1);
  }
  
  /**
   * 检查解码器是否已销毁
   */
  private checkDestroyed(): void {
    if (this.isDestroyed) {
      throw new Error('HLSv7Demuxer has been destroyed');
    }
  }
  
  /**
   * 判断是否应该记录日志
   * @param level 日志级别
   * @returns 是否应该记录
   */
  private shouldLog(level: LogLevel): boolean {
    if (this.logLevel === 'debug') {
      return true;
    }
    
    if (this.logLevel === 'info' && (level === 'info' || level === 'error' || level === 'warning')) {
      return true;
    }
    
    if (this.logLevel === 'error' && level === 'error') {
      return true;
    }
    
    return false;
  }
  
  /**
   * 记录日志
   * @param message 日志消息
   * @param level 日志级别
   */
  private log(message: string, level: LogLevel = 'debug'): void {
    if (this.shouldLog(level)) {
      console[level === 'warning' ? 'warn' : level](`[HLSv7Demuxer] ${message}`);
      
      // 发送日志事件到EventBus
      this.eventBus.emit(Events.LOG, message, level);
      
      // 触发调试事件
      if (this.eventHandlers?.debug && level === 'debug') {
        this.eventHandlers.debug(message);
      }
    }
  }
  
  /**
   * 触发自定义事件
   * @param event 事件名称
   * @param args 事件参数
   */
  private emitEvent(event: keyof HLSv7DemuxerEvents, ...args: any[]): void {
    const listeners = (this as any)._events?.[event];
    if (!listeners) return;
    
    if (Array.isArray(listeners)) {
      for (const listener of listeners) {
        listener.apply(this, args);
      }
    } else {
      listeners.apply(this, args);
    }
  }

  /**
   * 处理视频错误事件
   */
  private handleVideoError = (error: any): void => {
    this.log(`Video element error: ${error}`, 'error');
    
    // 当视频元素发生错误时，检查是否需要重试设置MediaSource
    if (this.isInitialized && error && error.code === 4) { // MEDIA_ERR_SRC_NOT_SUPPORTED
      const mediaSourceUrl = this.mediaSourceManager.getMediaSourceUrl();
      if (mediaSourceUrl) {
        this.log('Attempting to reattach MediaSource URL after video error', 'info');
        this.videoElementProxy.setSource(mediaSourceUrl);
      }
    }
  }

  /**
   * 处理编解码器请求
   * 返回当前使用的编解码器
   */
  private handleCodecRequest = (): string => {
    return this.codec;
  };

  /**
   * 获取第一个片段信息
   */
  private getFirstSegment(): MediaSegment | null {
    // 由于VirtualTimeline没有提供直接访问segments的公共方法
    // 我们使用findSegmentByIndex来获取第一个片段
    const segments = this.findAllSegments();
    return segments.length > 0 ? segments[0] : null;
  }

  /**
   * 设置初始化片段URL
   */
  private setInitSegmentUrl(url: string): void {
    if (this.segmentLoader && url) {
      // 通过重新初始化SegmentLoader来设置初始化片段URL
      const segments = this.findAllSegments();
      this.segmentLoader.initialize(segments, url);
    }
  }

  /**
   * 设置播放列表到虚拟时间轴
   */
  private setPlaylist(playlistInfo: PlaylistInfo): void {
    if (this.virtualTimeline && playlistInfo) {
      // 使用loadPlaylist方法设置播放列表
      this.virtualTimeline.loadPlaylist(playlistInfo);
    }
  }
}