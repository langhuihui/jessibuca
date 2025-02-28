/**
 * EventBus 集成测试
 * 测试 EventBus 在各个组件之间的通信，事件流的正确传递
 */

import { EventBus, Events } from '../EventBus';
import { VideoElementProxy } from '../VideoElementProxy';
import { SegmentLoader } from '../SegmentLoader';
import { MediaSourceManager } from '../MediaSourceManager';

// 创建一个模拟的 HTMLVideoElement
class MockVideoElement {
  src: string = '';
  currentTime: number = 0;
  duration: number = 0;
  paused: boolean = true;
  readyState: number = 0;
  buffered = {
    length: 0,
    start: (index: number) => 0,
    end: (index: number) => 0
  };
  
  addEventListener = jest.fn();
  removeEventListener = jest.fn();
  load = jest.fn();
  play = jest.fn().mockResolvedValue(undefined);
  pause = jest.fn();
}

describe('EventBus 组件间通信集成测试', () => {
  let eventBus: EventBus;
  let videoElementMock: MockVideoElement;
  let videoElementProxy: VideoElementProxy;
  let segmentLoader: SegmentLoader;
  let mediaSourceManager: MediaSourceManager;
  
  // 模拟事件监听器
  const mockVideoTimeUpdateHandler = jest.fn();
  const mockSegmentLoadedHandler = jest.fn();
  const mockBufferUpdateHandler = jest.fn();
  const mockErrorHandler = jest.fn();
  
  beforeEach(() => {
    // 创建新的事件总线和模拟对象
    eventBus = new EventBus();
    videoElementMock = new MockVideoElement();
    
    // 创建组件实例
    videoElementProxy = new VideoElementProxy(videoElementMock as unknown as HTMLVideoElement, eventBus);
    segmentLoader = new SegmentLoader(eventBus);
    mediaSourceManager = new MediaSourceManager(eventBus);
    
    // 设置测试监听器
    eventBus.on(Events.VIDEO_TIME_UPDATE, mockVideoTimeUpdateHandler);
    eventBus.on(Events.SEGMENT_LOADED, mockSegmentLoadedHandler);
    eventBus.on(Events.BUFFER_UPDATE, mockBufferUpdateHandler);
    eventBus.on(Events.ERROR, mockErrorHandler);
  });
  
  afterEach(() => {
    // 清理测试环境
    jest.clearAllMocks();
    if (videoElementProxy && typeof videoElementProxy.destroy === 'function') {
      videoElementProxy.destroy();
    }
    if (segmentLoader && typeof segmentLoader.destroy === 'function') {
      segmentLoader.destroy();
    }
    if (mediaSourceManager && typeof mediaSourceManager.destroy === 'function') {
      mediaSourceManager.destroy();
    }
  });
  
  test('视频时间更新事件应该被正确传递', () => {
    // 模拟视频元素的 timeupdate 事件
    const timeUpdateEvent = new Event('timeupdate');
    videoElementMock.currentTime = 10.5;
    
    // 手动调用 handleProgress 来模拟事件触发
    (videoElementProxy as any).handleProgress();
    
    // 验证事件是否被正确传递
    expect(mockVideoTimeUpdateHandler).toHaveBeenCalled();
  });
  
  test('片段加载事件应该被正确传递', () => {
    // 模拟片段加载完成
    const segmentIndex = 2;
    eventBus.emit(Events.SEGMENT_LOADED, segmentIndex);
    
    // 验证事件是否被正确传递给监听器
    expect(mockSegmentLoadedHandler).toHaveBeenCalledWith(segmentIndex);
  });
  
  test('缓冲区更新事件应该被正确传递', () => {
    // 模拟缓冲区更新
    const bufferRanges = [{ start: 0, end: 10 }];
    eventBus.emit(Events.BUFFER_UPDATE, bufferRanges);
    
    // 验证事件是否被正确传递给监听器
    expect(mockBufferUpdateHandler).toHaveBeenCalledWith(bufferRanges);
  });
  
  test('错误事件应该被正确传递', () => {
    // 模拟错误事件
    const error = new Error('Test error');
    eventBus.emit(Events.ERROR, 'Error message', error);
    
    // 验证事件是否被正确传递给监听器
    expect(mockErrorHandler).toHaveBeenCalledWith('Error message', error);
  });
  
  test('请求-响应模式应该正常工作', () => {
    // 创建一个返回特定值的请求处理器
    const mockCodecHandler = jest.fn().mockReturnValue('video/mp4; codecs="avc1.42E01E, mp4a.40.2"');
    eventBus.on(Events.CODEC_REQUEST, mockCodecHandler);
    
    // 使用request方法来获取结果
    const codecValue = eventBus.request(Events.CODEC_REQUEST);
    
    // 验证返回值
    expect(codecValue).toBe('video/mp4; codecs="avc1.42E01E, mp4a.40.2"');
    expect(mockCodecHandler).toHaveBeenCalled();
  });
}); 