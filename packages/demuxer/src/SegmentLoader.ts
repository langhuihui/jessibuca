import { MediaSegment } from './VirtualTimeline';
import { EventBus, Events } from './EventBus';

/**
 * 滑动窗口配置
 */
export interface SlidingWindowConfig {
  forward: number;  // 预加载前面的片段数
  backward: number; // 保留后面的片段数
  enabled: boolean; // 是否启用滑动窗口
}

/**
 * 缓冲区范围
 */
export interface BufferRange {
  start: number;
  end: number;
}

/**
 * SegmentLoader - 片段加载器
 * 
 * 负责媒体片段的加载、缓冲和管理
 * 通过事件总线与其他组件通信，不再直接依赖MediaSource和视频元素
 */
export class SegmentLoader {
  private eventBus: EventBus;
  
  // 滑动窗口配置
  private slidingWindowConfig: SlidingWindowConfig = {
    forward: 3,  // 预加载3个片段
    backward: 2, // 保留2个已播放片段
    enabled: true // 默认启用滑动窗口
  };
  
  // 片段状态
  private segments: MediaSegment[] = [];
  private currentSegmentIndex: number = -1;
  private segmentQueue: number[] = [];
  private pendingSegmentLoads: number[] = [];
  private isProcessingQueue: boolean = false;
  
  // 媒体资源状态
  private isMediaSourceReady: boolean = false;
  private fmp4InitSegmentUrl: string | null = null;
  private fmp4InitSegmentLoaded: boolean = false;
  
  // 缓冲区状态
  private bufferRanges: BufferRange[] = [];
  
  /**
   * 构造函数
   * @param eventBus 事件总线
   */
  constructor(eventBus: EventBus) {
    this.eventBus = eventBus;
    this.setupEventHandlers();
  }
  
  /**
   * 设置事件监听器
   */
  private setupEventHandlers(): void {
    // 监听MediaSource事件
    this.eventBus.on(Events.MEDIA_SOURCE_OPEN, this.handleMediaSourceOpen);
    this.eventBus.on(Events.MEDIA_SOURCE_CLOSE, this.handleMediaSourceClose);
    this.eventBus.on(Events.SOURCE_BUFFER_CREATED, this.handleSourceBufferCreated);
    this.eventBus.on(Events.SOURCE_BUFFER_UPDATE_END, this.handleSourceBufferUpdateEnd);
    
    // 监听片段和播放列表事件
    this.eventBus.on(Events.PLAYLIST_LOADED, this.handlePlaylistLoaded);
    this.eventBus.on(Events.SEGMENT_LOAD_START, this.handleSegmentLoadStart);
    this.eventBus.on(Events.TIMELINE_UPDATE, this.handleTimelineUpdate);
    this.eventBus.on(Events.TIMELINE_SEEK, this.handleTimelineSeek);
  }
  
  /**
   * 移除事件监听器
   */
  private removeEventHandlers(): void {
    this.eventBus.off(Events.MEDIA_SOURCE_OPEN, this.handleMediaSourceOpen);
    this.eventBus.off(Events.MEDIA_SOURCE_CLOSE, this.handleMediaSourceClose);
    this.eventBus.off(Events.SOURCE_BUFFER_CREATED, this.handleSourceBufferCreated);
    this.eventBus.off(Events.SOURCE_BUFFER_UPDATE_END, this.handleSourceBufferUpdateEnd);
    this.eventBus.off(Events.PLAYLIST_LOADED, this.handlePlaylistLoaded);
    this.eventBus.off(Events.SEGMENT_LOAD_START, this.handleSegmentLoadStart);
    this.eventBus.off(Events.TIMELINE_UPDATE, this.handleTimelineUpdate);
    this.eventBus.off(Events.TIMELINE_SEEK, this.handleTimelineSeek);
  }
  
  /**
   * 初始化片段加载器
   * @param segments 媒体片段列表
   * @param fmp4InitSegmentUrl FMP4初始化片段URL
   */
  public initialize(segments: MediaSegment[], fmp4InitSegmentUrl: string | null = null): void {
    this.segments = segments;
    this.fmp4InitSegmentUrl = fmp4InitSegmentUrl;
    this.currentSegmentIndex = -1;
    this.segmentQueue = [];
    this.pendingSegmentLoads = [];
    this.isProcessingQueue = false;
    this.bufferRanges = [];
    this.fmp4InitSegmentLoaded = false;
    
    // 记录初始化状态
    this.log(`Initialized SegmentLoader with ${segments.length} segments`, 'info');
    if (fmp4InitSegmentUrl) {
      this.log(`FMP4 init segment URL: ${fmp4InitSegmentUrl}`, 'info');
    }
    
    // 如果MediaSource已就绪，加载初始化片段
    if (this.isMediaSourceReady && fmp4InitSegmentUrl) {
      this.loadInitSegment();
    }
  }
  
  /**
   * 处理MediaSource打开事件
   */
  private handleMediaSourceOpen = (mediaSource: MediaSource): void => {
    this.isMediaSourceReady = true;
    this.log('MediaSource is now ready', 'info');
    
    // 如果有初始化片段URL，加载初始化片段
    if (this.fmp4InitSegmentUrl && !this.fmp4InitSegmentLoaded) {
      this.loadInitSegment();
    }
    
    // 处理待加载的片段
    this.processPendingSegmentLoads();
  }
  
  /**
   * 处理MediaSource关闭事件
   */
  private handleMediaSourceClose = (): void => {
    this.isMediaSourceReady = false;
    this.log('MediaSource is now closed', 'info');
  }
  
  /**
   * 处理SourceBuffer创建事件
   */
  private handleSourceBufferCreated = (sourceBuffer: SourceBuffer): void => {
    this.log('SourceBuffer has been created', 'info');
    
    // 处理待加载的片段
    this.processPendingSegmentLoads();
  }
  
  /**
   * 处理SourceBuffer更新结束事件
   */
  private handleSourceBufferUpdateEnd = (sourceBuffer: SourceBuffer): void => {
    // 更新缓冲区范围
    this.updateBufferRanges(sourceBuffer);
    
    // 继续处理队列中的下一个片段
    this.processNextSegment();
  }
  
  /**
   * 处理播放列表加载事件
   */
  private handlePlaylistLoaded = (playlistInfo: any): void => {
    this.log(`Playlist loaded with ${playlistInfo.segments.length} segments`, 'info');
    this.initialize(playlistInfo.segments, this.fmp4InitSegmentUrl);
  }
  
  /**
   * 处理片段加载开始事件
   */
  private handleSegmentLoadStart = (segmentIndex: number): void => {
    this.loadSegmentWithMSE(segmentIndex);
  }
  
  /**
   * 处理时间轴更新事件
   */
  private handleTimelineUpdate = (currentVirtualTime: number): void => {
    // 查找当前片段索引
    for (let i = 0; i < this.segments.length; i++) {
      const segment = this.segments[i];
      if (currentVirtualTime >= segment.virtualStartTime && currentVirtualTime < segment.virtualEndTime) {
        if (this.currentSegmentIndex !== i) {
          this.setCurrentSegmentIndex(i);
        }
        break;
      }
    }
  }
  
  /**
   * 处理时间轴跳转事件
   */
  private handleTimelineSeek = (seekTime: number): void => {
    // 查找跳转位置对应的片段索引
    for (let i = 0; i < this.segments.length; i++) {
      const segment = this.segments[i];
      if (seekTime >= segment.virtualStartTime && seekTime < segment.virtualEndTime) {
        this.setCurrentSegmentIndex(i);
        break;
      }
    }
  }
  
  /**
   * 设置滑动窗口配置
   * @param config 滑动窗口配置
   */
  public setSlidingWindowConfig(config: Partial<SlidingWindowConfig>): void {
    this.slidingWindowConfig = {
      ...this.slidingWindowConfig,
      ...config
    };
    
    this.log(`Updated sliding window config: forward=${this.slidingWindowConfig.forward}, backward=${this.slidingWindowConfig.backward}, enabled=${this.slidingWindowConfig.enabled}`, 'info');
    
    // 如果当前有活跃的片段索引，应用新的滑动窗口
    if (this.currentSegmentIndex >= 0) {
      this.applySegmentSlidingWindow(this.currentSegmentIndex);
    }
  }
  
  /**
   * 获取滑动窗口配置
   * @returns 滑动窗口配置
   */
  public getSlidingWindowConfig(): SlidingWindowConfig {
    return { ...this.slidingWindowConfig };
  }
  
  /**
   * 设置当前片段索引
   * @param index 片段索引
   */
  public setCurrentSegmentIndex(index: number): void {
    if (index < 0 || index >= this.segments.length || index === this.currentSegmentIndex) {
      return;
    }
    
    this.log(`Current segment index changed from ${this.currentSegmentIndex} to ${index}`, 'debug');
    this.currentSegmentIndex = index;
    
    // 应用滑动窗口
    if (this.slidingWindowConfig.enabled) {
      this.applySegmentSlidingWindow(index);
    }
    
    // 通知当前片段索引更改
    this.eventBus.emit(Events.CURRENT_SEGMENT_INDEX_CHANGE, index);
  }
  
  /**
   * 使用MSE加载片段
   * @param segmentIndex 片段索引
   */
  public loadSegmentWithMSE(segmentIndex: number): void {
    if (segmentIndex < 0 || segmentIndex >= this.segments.length) {
      this.log(`Invalid segment index: ${segmentIndex}`, 'error');
      return;
    }
    
    const segment = this.segments[segmentIndex];
    
    // 如果片段已经加载，跳过
    if (segment.isLoaded) {
      this.log(`Segment #${segmentIndex} already loaded, skipping`, 'debug');
      return;
    }
    
    // 如果片段正在加载，跳过
    if (segment.isLoading) {
      this.log(`Segment #${segmentIndex} already loading, skipping`, 'debug');
      return;
    }
    
    // 如果MediaSource未就绪，将片段添加到待处理队列
    if (!this.isMediaSourceReady) {
      this.log(`MediaSource not ready, adding segment #${segmentIndex} to pending queue`, 'debug');
      this.pendingSegmentLoads.push(segmentIndex);
      return;
    }
    
    // 将片段添加到加载队列
    this.enqueueSegment(segmentIndex);
  }
  
  /**
   * 加载所有片段
   */
  public loadAllSegments(): void {
    for (let i = 0; i < this.segments.length; i++) {
      this.loadSegmentWithMSE(i);
    }
  }
  
  /**
   * 清除所有片段
   */
  public clearAllSegments(): void {
    // 请求清除SourceBuffer
    this.eventBus.emit(Events.REQUEST_CLEAR_BUFFER);
    
    // 重置片段状态
    for (let i = 0; i < this.segments.length; i++) {
      const segment = this.segments[i];
      segment.isLoaded = false;
      segment.isLoading = false;
      segment.isBuffered = false;
      this.segments[i] = { ...segment };
    }
    
    // 清空队列
    this.segmentQueue = [];
    this.pendingSegmentLoads = [];
    this.isProcessingQueue = false;
    
    // 更新缓冲区范围
    this.bufferRanges = [];
    this.eventBus.emit(Events.BUFFER_UPDATE, this.bufferRanges);
    
    this.log('All segments cleared', 'info');
  }
  
  /**
   * 判断片段是否在滑动窗口内
   * @param segmentIndex 片段索引
   * @param currentIndex 当前片段索引
   * @returns 是否在滑动窗口内
   */
  public isSegmentInSlidingWindow(segmentIndex: number, currentIndex: number): boolean {
    if (!this.slidingWindowConfig.enabled) {
      return true;
    }
    
    // 计算片段与当前索引的距离
    const distance = segmentIndex - currentIndex;
    
    // 如果是未来片段，检查forward配置
    if (distance > 0) {
      return distance <= this.slidingWindowConfig.forward;
    }
    
    // 如果是过去片段，检查backward配置
    if (distance <= 0) {
      return Math.abs(distance) <= this.slidingWindowConfig.backward;
    }
    
    return true;
  }
  
  /**
   * 应用滑动窗口
   * @param currentIndex 当前片段索引
   */
  public applySegmentSlidingWindow(currentIndex: number): void {
    if (!this.slidingWindowConfig.enabled) {
      return;
    }
    
    // 检查每个片段，决定是否加载或移除
    for (let i = 0; i < this.segments.length; i++) {
      const isInWindow = this.isSegmentInSlidingWindow(i, currentIndex);
      
      if (isInWindow) {
        // 如果在窗口内且未加载，则加载
        if (!this.segments[i].isLoaded && !this.segments[i].isLoading) {
          this.loadSegmentWithMSE(i);
        }
      } else {
        // 如果不在窗口内但已加载，则考虑移除
        if (this.segments[i].isLoaded && this.segments[i].isBuffered) {
          // 计算与当前片段的距离
          const distance = Math.abs(i - currentIndex);
          
          // 只有当距离超过阈值时才移除
          if (i < currentIndex && distance > this.slidingWindowConfig.backward * 2) {
            // 请求移除片段
            this.eventBus.emit(Events.REQUEST_REMOVE_SEGMENT, i);
            
            // 更新片段状态
            const segment = this.segments[i];
            segment.isBuffered = false;
            this.segments[i] = { ...segment };
            
            this.log(`Removed segment #${i} from buffer (outside sliding window)`, 'debug');
            this.eventBus.emit(Events.SEGMENT_REMOVED, i);
          }
        }
      }
    }
  }
  
  /**
   * 将片段添加到加载队列
   * @param segmentIndex 片段索引
   */
  private enqueueSegment(segmentIndex: number): void {
    // 更新片段状态
    const segment = this.segments[segmentIndex];
    segment.isLoading = true;
    this.segments[segmentIndex] = { ...segment };
    
    // 添加到队列
    if (!this.segmentQueue.includes(segmentIndex)) {
      this.segmentQueue.push(segmentIndex);
      this.log(`Segment #${segmentIndex} queued for loading: ${segment.url}`, 'debug');
      this.eventBus.emit(Events.SEGMENT_QUEUED, segmentIndex);
    }
    
    // 如果队列未在处理中，开始处理
    if (!this.isProcessingQueue) {
      this.processNextSegment();
    }
  }
  
  /**
   * 处理队列中的下一个片段
   */
  private processNextSegment(): void {
    if (this.isProcessingQueue || this.segmentQueue.length === 0) {
      return;
    }
    
    this.isProcessingQueue = true;
    
    // 获取队列中的下一个片段
    const segmentIndex = this.segmentQueue.shift()!;
    const segment = this.segments[segmentIndex];
    
    this.log(`Starting to load segment #${segmentIndex}: ${segment.url}`, 'debug');
    this.eventBus.emit(Events.SEGMENT_LOAD_START, segmentIndex);
    
    // 请求加载片段数据
    this.fetchSegment(segment.url)
      .then(data => {
        // 请求将片段数据添加到SourceBuffer
        this.eventBus.emit(Events.APPEND_BUFFER, data, segmentIndex);
        
        // 更新片段状态
        segment.isLoaded = true;
        segment.isLoading = false;
        this.segments[segmentIndex] = { ...segment };
        
        this.log(`Segment #${segmentIndex} loaded successfully`, 'success');
        this.eventBus.emit(Events.SEGMENT_LOADED, segmentIndex);
        
        // 继续处理队列
        this.isProcessingQueue = false;
        this.processNextSegment();
      })
      .catch(error => {
        // 更新片段状态
        segment.isLoading = false;
        this.segments[segmentIndex] = { ...segment };
        
        this.log(`Failed to load segment #${segmentIndex}: ${error}`, 'error');
        this.eventBus.emit(Events.SEGMENT_LOAD_ERROR, segmentIndex, error);
        
        // 继续处理队列
        this.isProcessingQueue = false;
        this.processNextSegment();
      });
  }
  
  /**
   * 获取片段数据
   * @param url 片段URL
   * @returns 片段数据
   */
  private async fetchSegment(url: string): Promise<ArrayBuffer> {
    try {
      const response = await fetch(url);
      if (!response.ok) {
        throw new Error(`HTTP error! status: ${response.status}`);
      }
      
      return await response.arrayBuffer();
    } catch (error) {
      this.log(`Fetch error: ${error}`, 'error');
      throw error;
    }
  }
  
  /**
   * 处理待加载的片段
   */
  public processPendingSegmentLoads(): void {
    if (!this.isMediaSourceReady) {
      this.log('MediaSource not ready, cannot process pending segment loads', 'warning');
      return;
    }
    
    // 处理所有待加载的片段
    while (this.pendingSegmentLoads.length > 0) {
      const segmentIndex = this.pendingSegmentLoads.shift()!;
      this.loadSegmentWithMSE(segmentIndex);
    }
  }
  
  /**
   * 设置MediaSource就绪状态
   * @param isReady MediaSource是否就绪
   */
  public setMediaSourceReady(isReady: boolean): void {
    if (this.isMediaSourceReady === isReady) {
      return;
    }
    
    this.isMediaSourceReady = isReady;
    this.log(`MediaSource ready state changed to: ${isReady}`, 'debug');
    
    // 如果变为就绪状态，处理待加载的片段
    if (isReady) {
      this.processPendingSegmentLoads();
    }
  }
  
  /**
   * 加载初始化片段
   */
  private loadInitSegment(): void {
    if (!this.fmp4InitSegmentUrl || this.fmp4InitSegmentLoaded || !this.isMediaSourceReady) {
      return;
    }
    
    this.log(`Loading FMP4 init segment: ${this.fmp4InitSegmentUrl}`, 'info');
    
    this.fetchSegment(this.fmp4InitSegmentUrl)
      .then(data => {
        // 请求将初始化片段数据添加到SourceBuffer
        this.eventBus.emit(Events.APPEND_INIT_SEGMENT, data);
        
        this.fmp4InitSegmentLoaded = true;
        this.log('FMP4 init segment loaded successfully', 'success');
        
        // 处理待加载的片段
        this.processPendingSegmentLoads();
      })
      .catch(error => {
        this.log(`Failed to load FMP4 init segment: ${error}`, 'error');
        this.eventBus.emit(Events.ERROR, 'Init segment load error', error);
      });
  }
  
  /**
   * 更新缓冲区范围
   * @param sourceBuffer SourceBuffer对象
   */
  private updateBufferRanges(sourceBuffer: SourceBuffer): void {
    if (!sourceBuffer) {
      return;
    }
    
    const ranges: BufferRange[] = [];
    const buffered = sourceBuffer.buffered;
    
    for (let i = 0; i < buffered.length; i++) {
      ranges.push({
        start: buffered.start(i),
        end: buffered.end(i)
      });
    }
    
    this.bufferRanges = ranges;
    
    // 根据缓冲区范围更新片段的缓冲状态
    for (let i = 0; i < this.segments.length; i++) {
      const segment = this.segments[i];
      let isBuffered = false;
      
      // 检查片段是否在缓冲区范围内
      for (const range of ranges) {
        if (segment.virtualStartTime >= range.start && segment.virtualEndTime <= range.end) {
          isBuffered = true;
          break;
        }
      }
      
      // 更新片段缓冲状态
      if (segment.isBuffered !== isBuffered) {
        segment.isBuffered = isBuffered;
        this.segments[i] = { ...segment };
      }
    }
    
    // 通知缓冲区更新
    this.eventBus.emit(Events.BUFFER_UPDATE, this.bufferRanges);
    
    // 记录缓冲区范围
    if (ranges.length > 0) {
      const rangesStr = ranges.map(r => `${r.start.toFixed(2)}-${r.end.toFixed(2)}`).join(', ');
      this.log(`Buffer ranges updated: [${rangesStr}]`, 'debug');
    } else {
      this.log('Buffer is empty', 'debug');
    }
  }
  
  /**
   * 日志输出
   * @param message 日志消息
   * @param type 日志类型
   */
  private log(message: string, type: 'info' | 'success' | 'warning' | 'error' | 'debug'): void {
    this.eventBus.emit(Events.LOG, `[SegmentLoader] ${message}`, type);
  }
  
  /**
   * 销毁加载器，清理资源
   */
  public destroy(): void {
    this.removeEventHandlers();
    
    // 清空队列
    this.segmentQueue = [];
    this.pendingSegmentLoads = [];
    this.isProcessingQueue = false;
    
    // 重置状态
    this.segments = [];
    this.currentSegmentIndex = -1;
    this.isMediaSourceReady = false;
    this.fmp4InitSegmentUrl = null;
    this.fmp4InitSegmentLoaded = false;
    this.bufferRanges = [];
  }
} 