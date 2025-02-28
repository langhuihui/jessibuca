import { EventBus, Events } from "./EventBus";
const EXTINFRegex = /#EXTINF:(\d+\.\d+),(.*?),(.*?)\s*$/;

export interface MediaSegment {
  index: number;           // 片段索引
  url: string;             // 片段URL
  duration: number;        // 片段实际持续时间
  virtualStartTime: number; // 虚拟时间轴上的开始时间
  virtualEndTime: number;   // 虚拟时间轴上的结束时间
  physicalStartTime: number; // 物理时间轴上的开始时间
  physicalEndTime: number;   // 物理时间轴上的结束时间
  physicalTime: Date | null; // 物理时间（从EXTINF中解析）
  codec: string | null;     // 编解码器信息（从EXTINF中解析）
  isLoaded: boolean;        // 片段是否已加载
  isLoading: boolean;       // 片段是否正在加载
  isBuffered: boolean;      // 片段是否已缓冲
}

export interface BufferRange {
  start: number;
  end: number;
}

export interface PlaylistInfo {
  segments: MediaSegment[];
  totalDuration: number;
}

/**
 * VirtualTimeline - 虚拟时间轴管理器
 * 
 * 负责管理媒体片段在虚拟时间轴上的位置和状态
 * 提供虚拟时间和物理时间之间的转换
 * 通过事件总线与其他组件通信，不再直接依赖视频元素
 */
export class VirtualTimeline {
  private eventBus: EventBus;
  private segments: MediaSegment[] = [];
  private currentSegmentIndex: number = -1;
  private virtualTotalDuration: number = 0;
  private isPlaying: boolean = false;
  
  // 时间状态
  private currentVirtualTime: number = 0;
  private currentPhysicalTime: number = 0;
  
  // 缓冲区状态
  private bufferedRanges: BufferRange[] = [];
  
  // 配置参数
  private loadingDelay: number = 500; // 模拟加载延迟(毫秒)
  private playbackRate: number = 1;
  private playbackTimer: number | null = null;
  
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
    // 监听视频时间更新事件
    this.eventBus.on(Events.VIDEO_TIME_UPDATE, this.handleVideoTimeUpdate);
    
    // 监听视频播放控制事件
    this.eventBus.on(Events.VIDEO_PLAY, this.handleVideoPlay);
    this.eventBus.on(Events.VIDEO_PAUSE, this.handleVideoPause);
    
    // 监听视频跳转事件
    this.eventBus.on(Events.VIDEO_SEEKING, this.handleVideoSeeking);
    this.eventBus.on(Events.VIDEO_SEEKED, this.handleVideoSeeked);
    
    // 监听当前时间请求
    this.eventBus.on(Events.CURRENT_TIME_REQUEST, this.handleCurrentTimeRequest);
    this.eventBus.on(Events.TOTAL_DURATION_REQUEST, this.handleTotalDurationRequest);
  }
  
  /**
   * 移除事件监听器
   */
  private removeEventHandlers(): void {
    this.eventBus.off(Events.VIDEO_TIME_UPDATE, this.handleVideoTimeUpdate);
    this.eventBus.off(Events.VIDEO_PLAY, this.handleVideoPlay);
    this.eventBus.off(Events.VIDEO_PAUSE, this.handleVideoPause);
    this.eventBus.off(Events.VIDEO_SEEKING, this.handleVideoSeeking);
    this.eventBus.off(Events.VIDEO_SEEKED, this.handleVideoSeeked);
    this.eventBus.off(Events.CURRENT_TIME_REQUEST, this.handleCurrentTimeRequest);
    this.eventBus.off(Events.TOTAL_DURATION_REQUEST, this.handleTotalDurationRequest);
  }
  
  /**
   * 加载播放列表
   * @param playlist 播放列表信息
   */
  public loadPlaylist(playlist: PlaylistInfo): void {
    this.segments = playlist.segments;
    this.virtualTotalDuration = playlist.totalDuration;
    
    // 重置状态
    this.currentSegmentIndex = -1;
    this.currentVirtualTime = 0;
    this.currentPhysicalTime = 0;
    this.bufferedRanges = [];
    
    // 发送播放列表加载完成事件
    this.eventBus.emit(Events.LOG, `VirtualTimeline loaded playlist with ${this.segments.length} segments, total duration: ${this.virtualTotalDuration}s`, 'info');
    
    // 更新缓冲区范围
    this.updateBufferRanges();
    
    // 通知时间轴更新
    this.emitTimeUpdate();
  }
  
  /**
   * 从M3U8内容创建播放列表
   * @param m3u8Content M3U8文件内容
   * @param baseUrl 基础URL
   * @returns 播放列表信息
   */
  public createPlaylistFromM3U8(m3u8Content: string, baseUrl: string): PlaylistInfo {
    const lines = m3u8Content.split("\n");
    const segments: MediaSegment[] = [];
    let totalDuration = 0;
    let segmentIndex = 0;
    let segmentDuration = 0;
    let segmentTime: Date | null = null;
    let segmentCodec: string | null = null;
    
    for (let i = 0; i < lines.length; i++) {
      const line = lines[i].trim();
      
      // 解析EXTINF行
      if (line.startsWith("#EXTINF:")) {
        const match = line.match(EXTINFRegex);
        if (match) {
          segmentDuration = parseFloat(match[1]);
          const timeString = match[2] ? match[2].trim() : "";
          const codecString = match[3] ? match[3].trim() : "";
          
          // 解析物理时间
          try {
            if (timeString) {
              segmentTime = new Date(timeString);
            } else {
              segmentTime = null;
            }
          } catch (e) {
            segmentTime = null;
          }
          
          // 解析编解码器信息
          segmentCodec = codecString || null;
          
          this.eventBus.emit(Events.LOG, `解析EXTINF: 时长=${segmentDuration}秒, 物理时间=${segmentTime}, 编解码器=${segmentCodec}`, 'debug');
        }
      }
      // 处理片段URL行
      else if (!line.startsWith("#") && line !== "") {
        const url = this.resolveUrl(baseUrl, line);
        const virtualStartTime = totalDuration;
        const virtualEndTime = totalDuration + segmentDuration;
        const physicalStartTime = totalDuration;
        const physicalEndTime = totalDuration + segmentDuration;
        
        segments.push({
          index: segmentIndex,
          url,
          duration: segmentDuration,
          virtualStartTime,
          virtualEndTime,
          physicalStartTime,
          physicalEndTime,
          physicalTime: segmentTime,
          codec: segmentCodec,
          isLoaded: false,
          isLoading: false,
          isBuffered: false
        });
        
        totalDuration += segmentDuration;
        segmentIndex++;
        segmentTime = null;
        segmentCodec = null;
      }
    }
    
    return { segments, totalDuration };
  }
  
  /**
   * 解析URL
   * @param baseUrl 基础URL
   * @param relativeUrl 相对URL
   * @returns 完整URL
   */
  private resolveUrl(baseUrl: string, relativeUrl: string): string {
    if (relativeUrl.startsWith("http://") || relativeUrl.startsWith("https://")) {
      return relativeUrl;
    }
    
    // 删除baseUrl末尾的斜杠
    const base = baseUrl.endsWith("/") ? baseUrl.slice(0, -1) : baseUrl;
    // 删除relativeUrl开头的斜杠
    const relative = relativeUrl.startsWith("/") ? relativeUrl.slice(1) : relativeUrl;
    
    return `${base}/${relative}`;
  }
  
  /**
   * 查找指定虚拟时间对应的片段
   * @param virtualTime 虚拟时间
   * @returns 片段和片段内偏移
   */
  public findSegmentForVirtualTime(virtualTime: number): { segment: MediaSegment | null, offsetInSegment: number; } {
    if (this.segments.length === 0) {
      return { segment: null, offsetInSegment: 0 };
    }
    
    // 如果时间小于0，返回第一个片段
    if (virtualTime <= 0) {
      return { segment: this.segments[0], offsetInSegment: 0 };
    }
    
    // 如果时间大于总时长，返回最后一个片段
    if (virtualTime >= this.virtualTotalDuration) {
      const lastSegment = this.segments[this.segments.length - 1];
      return { segment: lastSegment, offsetInSegment: lastSegment.duration };
    }
    
    // 查找包含该虚拟时间的片段
    for (const segment of this.segments) {
      if (virtualTime >= segment.virtualStartTime && virtualTime < segment.virtualEndTime) {
        return { 
          segment, 
          offsetInSegment: virtualTime - segment.virtualStartTime 
        };
      }
    }
    
    return { segment: null, offsetInSegment: 0 };
  }
  
  /**
   * 虚拟时间转物理时间
   * @param virtualTime 虚拟时间
   * @returns 物理时间
   */
  public virtualToPhysicalTime(virtualTime: number): number {
    // 在不使用物理时间轴的情况下，虚拟时间等于物理时间
    return virtualTime;
  }
  
  /**
   * 物理时间转虚拟时间
   * @param physicalTime 物理时间
   * @returns 虚拟时间
   */
  public physicalToVirtualTime(physicalTime: number): number {
    // 在不使用物理时间轴的情况下，物理时间等于虚拟时间
    return physicalTime;
  }
  
  /**
   * 获取当前虚拟时间
   * @returns 当前虚拟时间
   */
  public getCurrentVirtualTime(): number {
    return this.currentVirtualTime;
  }
  
  /**
   * 获取总时长
   * @returns 总时长
   */
  public getTotalDuration(): number {
    return this.virtualTotalDuration;
  }
  
  /**
   * 获取当前播放进度(0-1)
   * @returns 当前播放进度
   */
  public getProgress(): number {
    if (this.virtualTotalDuration <= 0) {
      return 0;
    }
    return this.currentVirtualTime / this.virtualTotalDuration;
  }
  
  /**
   * 开始播放
   */
  public play(): void {
    if (this.isPlaying) return;
    
    this.isPlaying = true;
    this.startPlaybackTimer();
    
    // 通知播放状态变化
    this.eventBus.emit(Events.PLAYBACK_STATE_CHANGE, this.isPlaying);
    this.eventBus.emit(Events.LOG, 'VirtualTimeline playback started', 'debug');
  }
  
  /**
   * 暂停播放
   */
  public pause(): void {
    if (!this.isPlaying) return;
    
    this.isPlaying = false;
    this.stopPlaybackTimer();
    
    // 通知播放状态变化
    this.eventBus.emit(Events.PLAYBACK_STATE_CHANGE, this.isPlaying);
    this.eventBus.emit(Events.LOG, 'VirtualTimeline playback paused', 'debug');
  }
  
  /**
   * 跳转到指定时间
   * @param virtualTime 虚拟时间
   */
  public seek(virtualTime: number): void {
    // 边界检查
    if (virtualTime < 0) {
      virtualTime = 0;
    } else if (virtualTime > this.virtualTotalDuration) {
      virtualTime = this.virtualTotalDuration;
    }
    
    const prevTime = this.currentVirtualTime;
    this.currentVirtualTime = virtualTime;
    this.currentPhysicalTime = this.virtualToPhysicalTime(virtualTime);
    
    // 查找对应片段
    const { segment } = this.findSegmentForVirtualTime(virtualTime);
    if (segment) {
      this.currentSegmentIndex = segment.index;
    }
    
    // 触发seeking事件
    this.eventBus.emit(Events.TIMELINE_SEEK, virtualTime, this.currentPhysicalTime);
    this.eventBus.emit(Events.LOG, `VirtualTimeline seeking from ${prevTime}s to ${virtualTime}s`, 'debug');
    
    // 如果跳转到未加载的片段，则加载该片段及其周围的片段
    this.loadSegmentsForTime(virtualTime);
    
    // 触发seeked事件
    this.eventBus.emit(Events.TIMELINE_SEEKED, virtualTime, this.currentPhysicalTime);
    
    // 触发时间更新事件
    this.emitTimeUpdate();
  }
  
  /**
   * 加载指定时间点所需的片段
   * @param virtualTime 虚拟时间
   */
  private async loadSegmentsForTime(virtualTime: number): Promise<void> {
    const { segment } = this.findSegmentForVirtualTime(virtualTime);
    if (!segment) return;
    
    // 加载当前片段
    if (!segment.isLoaded && !segment.isLoading) {
      await this.loadSegmentByIndex(segment.index);
    }
    
    // 预加载下一个片段
    this.preloadNextSegment();
  }
  
  /**
   * 通过索引加载片段
   * @param index 片段索引
   */
  private async loadSegmentByIndex(index: number): Promise<void> {
    if (index < 0 || index >= this.segments.length) return;
    
    const segment = this.segments[index];
    
    // 如果已经加载或正在加载，则跳过
    if (segment.isLoaded || segment.isLoading) return;
    
    // 标记为正在加载
    segment.isLoading = true;
    this.segments[index] = { ...segment };
    
    // 触发加载开始事件
    this.eventBus.emit(Events.SEGMENT_LOAD_START, segment);
    this.eventBus.emit(Events.LOG, `Loading segment #${index}: ${segment.url}`, 'debug');
    
    try {
      // 通知SegmentLoader加载该片段
      this.eventBus.emit(Events.SEGMENT_LOAD_START, index);
      
      // 模拟加载延迟
      await new Promise(resolve => setTimeout(resolve, this.loadingDelay));
      
      // 标记为已加载
      segment.isLoaded = true;
      segment.isLoading = false;
      this.segments[index] = { ...segment };
      
      // 更新缓冲区状态
      this.updateBufferRanges();
      
      // 触发加载完成事件
      this.eventBus.emit(Events.SEGMENT_LOADED, segment);
      this.eventBus.emit(Events.LOG, `Segment #${index} loaded successfully`, 'success');
    } catch (error) {
      // 标记为加载失败
      segment.isLoading = false;
      this.segments[index] = { ...segment };
      
      // 触发加载错误事件
      this.eventBus.emit(Events.SEGMENT_LOAD_ERROR, segment, error);
      this.eventBus.emit(Events.LOG, `Failed to load segment #${index}: ${error}`, 'error');
    }
  }
  
  /**
   * 更新缓冲区范围
   */
  private updateBufferRanges(): void {
    const ranges: BufferRange[] = [];
    let currentStart: number | null = null;
    let currentEnd: number | null = null;
    
    // 根据已加载片段构建缓冲区范围
    for (let i = 0; i < this.segments.length; i++) {
      const segment = this.segments[i];
      
      if (segment.isLoaded) {
        // 标记为已缓冲
        segment.isBuffered = true;
        this.segments[i] = { ...segment };
        
        // 如果是新范围的开始
        if (currentStart === null) {
          currentStart = segment.virtualStartTime;
          currentEnd = segment.virtualEndTime;
        }
        // 如果是现有范围的延续
        else if (currentEnd === segment.virtualStartTime) {
          currentEnd = segment.virtualEndTime;
        }
        // 如果是不连续的新范围
        else {
          // 保存之前的范围
          if (currentStart !== null && currentEnd !== null) {
            ranges.push({ start: currentStart, end: currentEnd });
          }
          // 开始新的范围
          currentStart = segment.virtualStartTime;
          currentEnd = segment.virtualEndTime;
        }
      }
    }
    
    // 添加最后一个范围
    if (currentStart !== null && currentEnd !== null) {
      ranges.push({ start: currentStart, end: currentEnd });
    }
    
    this.bufferedRanges = ranges;
    
    // 触发缓冲区更新事件
    this.eventBus.emit(Events.BUFFER_UPDATE, this.bufferedRanges);
    this.eventBus.emit(Events.BUFFER_UPDATE, this.bufferedRanges);
  }
  
  /**
   * 预加载下一个片段
   */
  private preloadNextSegment(): void {
    const nextIndex = this.currentSegmentIndex + 1;
    if (nextIndex < this.segments.length) {
      this.loadSegmentByIndex(nextIndex);
    }
  }
  
  /**
   * 启动播放定时器
   */
  private startPlaybackTimer(): void {
    if (this.playbackTimer !== null) {
      this.stopPlaybackTimer();
    }
    
    const updateInterval = 100; // 100ms更新一次
    let lastUpdateTime = Date.now();
    
    this.playbackTimer = window.setInterval(() => {
      const now = Date.now();
      const deltaMs = now - lastUpdateTime;
      lastUpdateTime = now;
      
      // 按播放速率更新虚拟时间
      const deltaSeconds = (deltaMs / 1000) * this.playbackRate;
      this.currentVirtualTime += deltaSeconds;
      this.currentPhysicalTime = this.virtualToPhysicalTime(this.currentVirtualTime);
      
      // 检查是否超出总时长
      if (this.currentVirtualTime >= this.virtualTotalDuration) {
        this.currentVirtualTime = this.virtualTotalDuration;
        this.pause();
      }
      
      // 更新当前片段索引
      const { segment } = this.findSegmentForVirtualTime(this.currentVirtualTime);
      if (segment) {
        this.currentSegmentIndex = segment.index;
      }
      
      // 预加载下一个片段
      this.preloadNextSegment();
      
      // 触发时间更新事件
      this.emitTimeUpdate();
    }, updateInterval);
  }
  
  /**
   * 停止播放定时器
   */
  private stopPlaybackTimer(): void {
    if (this.playbackTimer !== null) {
      clearInterval(this.playbackTimer);
      this.playbackTimer = null;
    }
  }
  
  /**
   * 设置播放速率
   * @param rate 播放速率
   */
  public setPlaybackRate(rate: number): void {
    this.playbackRate = rate;
    
    // 通过事件总线通知播放速率变化
    this.eventBus.emit(Events.PLAYBACK_RATE_CHANGE, rate);
    this.eventBus.emit(Events.LOG, `Playback rate set to ${rate}x`, 'debug');
    
    // 如果正在播放，重新启动定时器以应用新的播放速率
    if (this.isPlaying) {
      this.stopPlaybackTimer();
      this.startPlaybackTimer();
    }
  }
  
  /**
   * 发送时间更新事件
   */
  private emitTimeUpdate(): void {
    this.eventBus.emit(Events.TIMELINE_UPDATE, this.currentVirtualTime, this.currentPhysicalTime);
    this.eventBus.emit(Events.TIMELINE_UPDATE, this.currentVirtualTime);
  }
  
  /**
   * 处理视频时间更新事件
   */
  private handleVideoTimeUpdate = (currentTime: number): void => {
    // 将视频当前时间转换为虚拟时间
    this.currentVirtualTime = this.physicalToVirtualTime(currentTime);
    
    // 如果虚拟播放时间不更新，则使用视频时间作为备用
    if (this.currentVirtualTime === 0 && currentTime > 0) {
      this.currentVirtualTime = currentTime;
    }
    
    // 更新当前片段索引
    const { segment } = this.findSegmentForVirtualTime(this.currentVirtualTime);
    if (segment) {
      this.currentSegmentIndex = segment.index;
    }
    
    // 触发时间更新事件，通知其他组件
    this.emitTimeUpdate();
  }
  
  /**
   * 处理视频播放事件
   */
  private handleVideoPlay = (): void => {
    this.isPlaying = true;
    
    // 停止内部播放定时器，依赖视频元素的时间更新
    this.stopPlaybackTimer();
    
    // 通知播放状态变化
    this.eventBus.emit(Events.PLAYBACK_STATE_CHANGE, this.isPlaying);
  }
  
  /**
   * 处理视频暂停事件
   */
  private handleVideoPause = (): void => {
    this.isPlaying = false;
    
    // 通知播放状态变化
    this.eventBus.emit(Events.PLAYBACK_STATE_CHANGE, this.isPlaying);
  }
  
  /**
   * 处理视频seeking事件
   */
  private handleVideoSeeking = (time: number): void => {
    // 将物理时间转换为虚拟时间
    const virtualTime = this.physicalToVirtualTime(time);
    
    // 触发seeking事件
    this.eventBus.emit(Events.TIMELINE_SEEK, virtualTime, time);
    
    // 加载所需片段
    this.loadSegmentsForTime(virtualTime);
  }
  
  /**
   * 处理视频seeked事件
   */
  private handleVideoSeeked = (time: number): void => {
    // 将物理时间转换为虚拟时间
    const virtualTime = this.physicalToVirtualTime(time);
    
    // 更新当前时间
    this.currentVirtualTime = virtualTime;
    this.currentPhysicalTime = time;
    
    // 触发seeked事件
    this.eventBus.emit(Events.TIMELINE_SEEKED, virtualTime, time);
    
    // 更新时间
    this.emitTimeUpdate();
  }
  
  /**
   * 处理当前时间请求
   */
  private handleCurrentTimeRequest = (): number => {
    return this.currentVirtualTime;
  }
  
  /**
   * 处理总时长请求
   */
  private handleTotalDurationRequest = (): number => {
    return this.virtualTotalDuration;
  }
  
  /**
   * 销毁时间轴，清理资源
   */
  public destroy(): void {
    this.stopPlaybackTimer();
    this.removeEventHandlers();
    
    // 重置状态
    this.segments = [];
    this.currentSegmentIndex = -1;
    this.virtualTotalDuration = 0;
    this.isPlaying = false;
    this.currentVirtualTime = 0;
    this.currentPhysicalTime = 0;
    this.bufferedRanges = [];
  }
} 