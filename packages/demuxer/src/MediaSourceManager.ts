import { EventBus, Events } from './EventBus';

/**
 * MediaSourceManager - MediaSource管理器
 * 负责创建和管理MediaSource对象的生命周期
 */
export class MediaSourceManager {
  private eventBus: EventBus;
  private mediaSource: MediaSource | null = null;
  private sourceBuffers: Record<string, SourceBuffer> = {};
  private isReady: boolean = false;
  private sourceOpenTimeout: number | null = null;
  private mediaSourceUrl: string | null = null;
  private retryCount: number = 0;
  private maxRetries: number = 2; // 最大重试次数
  
  /**
   * 构造函数
   * @param eventBus 事件总线
   */
  constructor(eventBus: EventBus) {
    this.eventBus = eventBus;
    
    // 注册codec请求处理
    this.eventBus.on(Events.CODEC_REQUEST, this.handleCodecRequest);
  }
  
  /**
   * 创建新的MediaSource对象
   * @returns 返回创建的MediaSource URL
   */
  public createMediaSource(): string {
    this.cleanup();
    
    this.mediaSource = new MediaSource();
    this.setupMediaSourceEvents();
    
    const url = URL.createObjectURL(this.mediaSource);
    this.mediaSourceUrl = url;
    this.log('Created MediaSource URL: ' + url, 'debug');
    
    // 设置超时检测，防止sourceopen事件未触发
    this.setupSourceOpenTimeout();
    this.retryCount = 0;
    
    return url;
  }
  
  /**
   * 获取当前MediaSource URL
   */
  public getMediaSourceUrl(): string | null {
    return this.mediaSourceUrl;
  }
  
  /**
   * 设置MediaSource事件监听
   */
  private setupMediaSourceEvents(): void {
    if (!this.mediaSource) return;
    
    this.mediaSource.addEventListener('sourceopen', this.handleSourceOpen);
    this.mediaSource.addEventListener('sourceended', this.handleSourceEnded);
    this.mediaSource.addEventListener('sourceclose', this.handleSourceClose);
  }
  
  /**
   * 设置sourceopen事件超时检测
   */
  private setupSourceOpenTimeout(): void {
    if (this.sourceOpenTimeout !== null) {
      clearTimeout(this.sourceOpenTimeout);
    }
    
    this.sourceOpenTimeout = window.setTimeout(() => {
      if (!this.mediaSource) return;
      
      if (this.mediaSource.readyState === 'open') {
        this.log('MediaSource \'sourceopen\' event timed out after 5s, but readyState is already open', 'debug');
        // 确保我们触发open事件，即使浏览器没有触发
        this.eventBus.emit(Events.MEDIA_SOURCE_OPEN, this.mediaSource);
      } else {
        this.log('MediaSource \'sourceopen\' event timed out after 5s, readyState: ' + this.mediaSource.readyState, 'error');
        
        // 尝试恢复 - 如果还有重试次数，重新创建MediaSource
        if (this.retryCount < this.maxRetries) {
          this.retryCount++;
          this.log(`Retrying MediaSource creation (attempt ${this.retryCount}/${this.maxRetries})`, 'info');
          
          // 保存当前URL以便释放
          const oldUrl = this.mediaSourceUrl;
          
          // 重新创建MediaSource
          const newUrl = this.createMediaSource();
          
          // 通知需要更新视频源
          this.eventBus.emit('video:updateSource', newUrl);
          
          // 释放旧URL
          if (oldUrl) {
            try {
              URL.revokeObjectURL(oldUrl);
            } catch (e) {
              // 忽略错误
            }
          }
        } else {
          this.eventBus.emit(Events.ERROR, 'MediaSource sourceopen timeout', { readyState: this.mediaSource.readyState });
        }
      }
      
      this.sourceOpenTimeout = null;
    }, 5000);
  }
  
  /**
   * 手动触发sourceopen过程 - 用于特殊情况下强制初始化
   */
  public forceSourceOpen(): void {
    if (!this.mediaSource) {
      this.log('Cannot force source open, MediaSource not created', 'error');
      return;
    }
    
    if (this.mediaSource.readyState === 'open') {
      this.log('MediaSource is already open, triggering event manually', 'info');
      this.handleSourceOpen();
    } else {
      this.log('Attempting to force MediaSource open, current readyState: ' + this.mediaSource.readyState, 'warning');
      
      // 创建新的MediaSource
      const newUrl = this.createMediaSource();
      
      // 通知需要更新视频源
      this.eventBus.emit('video:updateSource', newUrl);
    }
  }
  
  /**
   * 处理MediaSource的sourceopen事件
   */
  private handleSourceOpen = (): void => {
    if (!this.mediaSource) return;
    
    // 清除超时计时器
    if (this.sourceOpenTimeout !== null) {
      clearTimeout(this.sourceOpenTimeout);
      this.sourceOpenTimeout = null;
    }
    
    this.isReady = true;
    this.retryCount = 0; // 重置重试计数
    this.log('MediaSource sourceopen event fired, readyState: ' + this.mediaSource.readyState, 'debug');
    this.eventBus.emit(Events.MEDIA_SOURCE_OPEN, this.mediaSource);
  }
  
  /**
   * 处理MediaSource的sourceended事件
   */
  private handleSourceEnded = (): void => {
    if (!this.mediaSource) return;
    this.log('MediaSource sourceended event fired, readyState: ' + this.mediaSource.readyState, 'debug');
  }
  
  /**
   * 处理MediaSource的sourceclose事件
   */
  private handleSourceClose = (): void => {
    if (!this.mediaSource) return;
    this.isReady = false;
    this.log('MediaSource sourceclose event fired, readyState: ' + this.mediaSource.readyState, 'debug');
    this.eventBus.emit(Events.MEDIA_SOURCE_CLOSE, this.mediaSource);
  }
  
  /**
   * 创建SourceBuffer
   * @param mimeType MIME类型
   * @param id SourceBuffer标识符
   * @returns 创建的SourceBuffer或null
   */
  public createSourceBuffer(mimeType: string, id: string = 'default'): SourceBuffer | null {
    if (!this.mediaSource || this.mediaSource.readyState !== 'open') {
      this.log('Cannot create SourceBuffer, MediaSource not ready', 'error');
      return null;
    }
    
    try {
      const sourceBuffer = this.mediaSource.addSourceBuffer(mimeType);
      this.sourceBuffers[id] = sourceBuffer;
      this.setupSourceBufferEvents(sourceBuffer, id);
      
      this.log(`SourceBuffer created successfully with codec: ${mimeType}`, 'debug');
      this.eventBus.emit(Events.SOURCE_BUFFER_CREATED, sourceBuffer, id);
      
      return sourceBuffer;
    } catch (error) {
      this.log(`Error creating SourceBuffer: ${error}`, 'error');
      this.eventBus.emit(Events.ERROR, 'SourceBuffer creation error', error);
      return null;
    }
  }
  
  /**
   * 设置SourceBuffer事件监听
   */
  private setupSourceBufferEvents(sourceBuffer: SourceBuffer, id: string): void {
    sourceBuffer.addEventListener('updateend', () => {
      this.eventBus.emit(Events.SOURCE_BUFFER_UPDATE_END, sourceBuffer, id);
    });
    
    sourceBuffer.addEventListener('error', (e) => {
      this.log(`SourceBuffer error: ${e}`, 'error');
      this.eventBus.emit(Events.ERROR, 'SourceBuffer error', e);
    });
  }
  
  /**
   * 获取SourceBuffer
   * @param id SourceBuffer标识符
   * @returns SourceBuffer或null
   */
  public getSourceBuffer(id: string = 'default'): SourceBuffer | null {
    return this.sourceBuffers[id] || null;
  }
  
  /**
   * 处理编解码器请求
   * @returns 当前活跃的编解码器信息
   */
  private handleCodecRequest = (): string | null => {
    // 如果有活跃的SourceBuffer，则返回其mimeType
    const sourceBuffer = this.getSourceBuffer();
    if (sourceBuffer) {
      return sourceBuffer.mode;
    }
    return null;
  }
  
  /**
   * 检查MediaSource是否准备就绪
   */
  public isMediaSourceReady(): boolean {
    return this.isReady && this.mediaSource?.readyState === 'open';
  }
  
  /**
   * 获取MediaSource对象
   */
  public getMediaSource(): MediaSource | null {
    return this.mediaSource;
  }
  
  /**
   * 清理资源
   */
  public cleanup(): void {
    if (this.sourceOpenTimeout !== null) {
      clearTimeout(this.sourceOpenTimeout);
      this.sourceOpenTimeout = null;
    }
    
    if (this.mediaSource) {
      if (this.mediaSource.readyState === 'open') {
        try {
          this.mediaSource.endOfStream();
        } catch (e) {
          // 忽略可能的错误
        }
      }
      
      this.mediaSource.removeEventListener('sourceopen', this.handleSourceOpen);
      this.mediaSource.removeEventListener('sourceended', this.handleSourceEnded);
      this.mediaSource.removeEventListener('sourceclose', this.handleSourceClose);
    }
    
    // 释放当前URL
    if (this.mediaSourceUrl) {
      try {
        URL.revokeObjectURL(this.mediaSourceUrl);
        this.mediaSourceUrl = null;
      } catch (e) {
        // 忽略错误
      }
    }
    
    this.sourceBuffers = {};
    this.mediaSource = null;
    this.isReady = false;
  }
  
  /**
   * 销毁管理器
   */
  public destroy(): void {
    this.cleanup();
    this.eventBus.off(Events.CODEC_REQUEST, this.handleCodecRequest);
  }
  
  /**
   * 日志输出
   */
  private log(message: string, level: 'debug' | 'info' | 'error' | 'warning' = 'debug'): void {
    this.eventBus.emit(Events.LOG, message, level);
  }
} 