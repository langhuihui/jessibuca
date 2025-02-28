/**
 * EventBus - 组件间通信的中心枢纽
 * 实现松耦合的事件驱动架构，减少组件间的直接依赖
 */

type EventCallback = (...args: any[]) => void;

export class EventBus {
  private listeners: Record<string, EventCallback[]> = {};
  
  /**
   * 订阅事件
   * @param event 事件名称
   * @param callback 回调函数
   */
  on(event: string, callback: EventCallback): void {
    if (!this.listeners[event]) {
      this.listeners[event] = [];
    }
    this.listeners[event].push(callback);
  }

  /**
   * 取消事件订阅
   * @param event 事件名称
   * @param callback 回调函数
   */
  off(event: string, callback: EventCallback): void {
    if (!this.listeners[event]) return;
    this.listeners[event] = this.listeners[event].filter(cb => cb !== callback);
  }

  /**
   * 触发事件
   * @param event 事件名称
   * @param args 事件参数
   */
  emit(event: string, ...args: any[]): void {
    if (!this.listeners[event]) return;
    this.listeners[event].forEach(callback => callback(...args));
  }

  /**
   * 触发事件并返回结果（用于请求-响应模式）
   * @param event 事件名称
   * @param args 事件参数
   * @returns 第一个监听器的返回值
   */
  request(event: string, ...args: any[]): any {
    if (!this.listeners[event] || this.listeners[event].length === 0) return null;
    // 只返回第一个监听器的结果
    return this.listeners[event][0](...args);
  }

  /**
   * 清除所有事件监听器
   */
  clear(): void {
    this.listeners = {};
  }
}

/**
 * 标准事件名称常量
 */
export const Events = {
  // MediaSource 相关事件
  MEDIA_SOURCE_OPEN: 'mediaSource:open',
  MEDIA_SOURCE_CLOSE: 'mediaSource:close',
  SOURCE_BUFFER_CREATED: 'sourceBuffer:created',
  SOURCE_BUFFER_UPDATE_END: 'sourceBuffer:updateEnd',
  
  // 视频元素事件
  VIDEO_TIME_UPDATE: 'video:timeUpdate',
  VIDEO_PLAY: 'video:play',
  VIDEO_PAUSE: 'video:pause',
  VIDEO_SEEKING: 'video:seeking',
  VIDEO_SEEKED: 'video:seeked',
  VIDEO_WAITING: 'video:waiting',
  VIDEO_CANPLAY: 'video:canPlay',
  VIDEO_PROGRESS: 'video:progress',
  VIDEO_LOADED_METADATA: 'video:loadedMetadata',
  VIDEO_ERROR: 'video:error',
  
  // 播放列表和片段相关事件
  PLAYLIST_LOADED: 'playlist:loaded',
  SEGMENT_LOAD_START: 'segment:loadStart',
  SEGMENT_LOADED: 'segment:loaded',
  SEGMENT_LOAD_ERROR: 'segment:loadError',
  SEGMENT_REMOVED: 'segment:removed',
  SEGMENT_QUEUED: 'segment:queued',
  CURRENT_SEGMENT_INDEX_CHANGE: 'segment:currentIndexChange',
  
  // 时间轴相关事件
  TIMELINE_UPDATE: 'timeline:update', 
  TIMELINE_SEEK: 'timeline:seek',
  TIMELINE_SEEKED: 'timeline:seeked',
  
  // 缓冲区管理事件
  BUFFER_UPDATE: 'buffer:update',
  BUFFER_FULL: 'buffer:full',
  BUFFER_EMPTY: 'buffer:empty',
  BUFFER_FLUSHING: 'buffer:flushing',
  BUFFER_FLUSHED: 'buffer:flushed',
  REQUEST_CLEAR_BUFFER: 'buffer:requestClear',
  REQUEST_REMOVE_SEGMENT: 'buffer:requestRemoveSegment',
  
  // 错误和日志事件
  ERROR: 'error',
  LOG: 'log',
  
  // 请求事件
  CODEC_REQUEST: 'request:codec',
  CURRENT_TIME_REQUEST: 'request:currentTime',
  TOTAL_DURATION_REQUEST: 'request:totalDuration',
  
  // 数据事件
  APPEND_BUFFER: 'data:appendBuffer',
  APPEND_INIT_SEGMENT: 'data:appendInitSegment',
  
  // 状态事件
  MEDIA_ATTACHED: 'media:attached',
  MEDIA_DETACHED: 'media:detached',
  PLAYBACK_STATE_CHANGE: 'playback:stateChange',
  PLAYBACK_RATE_CHANGE: 'playback:rateChange',
}; 