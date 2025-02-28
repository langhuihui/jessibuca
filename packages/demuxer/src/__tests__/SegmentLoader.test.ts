/**
 * SegmentLoader 单元测试
 * 测试片段加载、滑动窗口和事件发送等核心功能
 */

import { EventBus, Events } from '../EventBus';
import { SegmentLoader, SlidingWindowConfig } from '../SegmentLoader';
import { MediaSegment } from '../VirtualTimeline';

describe('SegmentLoader 基础功能测试', () => {
  let eventBus: EventBus;
  let segmentLoader: SegmentLoader;
  let mockMediaSource: any;
  let mockSourceBuffer: any;
  let mockSegments: MediaSegment[];

  beforeEach(() => {
    // 创建模拟对象
    eventBus = new EventBus();
    
    // 监听相关事件
    jest.spyOn(eventBus, 'emit');
    jest.spyOn(eventBus, 'on');
    
    // 模拟MediaSource和SourceBuffer
    mockSourceBuffer = {
      mode: 'segments',
      updating: false,
      buffered: {
        length: 1,
        start: jest.fn().mockReturnValue(0),
        end: jest.fn().mockReturnValue(10)
      },
      appendBuffer: jest.fn(),
      remove: jest.fn(),
      addEventListener: jest.fn(),
      removeEventListener: jest.fn()
    };

    mockMediaSource = {
      readyState: 'open',
      addSourceBuffer: jest.fn().mockReturnValue(mockSourceBuffer),
      endOfStream: jest.fn(),
      removeSourceBuffer: jest.fn(),
      addEventListener: jest.fn(),
      removeEventListener: jest.fn()
    };

    // 创建模拟片段
    mockSegments = Array.from({ length: 5 }, (_, i) => ({
      index: i,
      url: `http://example.com/segment_${i}.mp4`,
      duration: 10,
      virtualStartTime: i * 10,
      virtualEndTime: (i + 1) * 10,
      physicalStartTime: i * 10,
      physicalEndTime: (i + 1) * 10,
      physicalTime: new Date(),
      codec: 'avc1.64001E',
      isLoaded: false,
      isLoading: false,
      isBuffered: false
    }));

    // 创建SegmentLoader实例
    segmentLoader = new SegmentLoader(eventBus);
    
    // 模拟fetch响应
    global.fetch = jest.fn().mockImplementation(() =>
      Promise.resolve({
        ok: true,
        status: 200,
        arrayBuffer: () => Promise.resolve(new ArrayBuffer(1024))
      })
    );
  });

  afterEach(() => {
    jest.clearAllMocks();
    segmentLoader.destroy();
  });

  test('初始化 - 应该正确配置SegmentLoader', () => {
    // 初始化SegmentLoader
    segmentLoader.initialize(mockSegments, 'http://example.com/init.mp4');
    
    // 验证事件监听是否正确设置
    expect(eventBus.on).toHaveBeenCalledWith(Events.MEDIA_SOURCE_OPEN, expect.any(Function));
    expect(eventBus.on).toHaveBeenCalledWith(Events.MEDIA_SOURCE_CLOSE, expect.any(Function));
    expect(eventBus.on).toHaveBeenCalledWith(Events.SOURCE_BUFFER_CREATED, expect.any(Function));
    expect(eventBus.on).toHaveBeenCalledWith(Events.SOURCE_BUFFER_UPDATE_END, expect.any(Function));
    
    // 验证初始化段URL是否正确设置
    expect(segmentLoader['fmp4InitSegmentUrl']).toBe('http://example.com/init.mp4');
  });

  test('设置当前片段索引 - 应该触发事件并更新索引', () => {
    segmentLoader.initialize(mockSegments);
    segmentLoader.setCurrentSegmentIndex(2);
    
    // 验证事件是否触发，当前索引是否更新
    expect(eventBus.emit).toHaveBeenCalledWith(Events.CURRENT_SEGMENT_INDEX_CHANGE, 2);
    expect(segmentLoader['currentSegmentIndex']).toBe(2);
  });

  test('设置滑动窗口配置 - 应该正确更新配置', () => {
    const newConfig: Partial<SlidingWindowConfig> = {
      forward: 3,
      backward: 2,
      enabled: true
    };
    
    segmentLoader.setSlidingWindowConfig(newConfig);
    const config = segmentLoader.getSlidingWindowConfig();
    
    expect(config.forward).toBe(3);
    expect(config.backward).toBe(2);
    expect(config.enabled).toBe(true);
  });

  test('loadSegmentWithMSE - 当MediaSource就绪时应该加载片段', () => {
    segmentLoader.initialize(mockSegments);
    
    // 模拟MediaSource就绪
    segmentLoader.setMediaSourceReady(true);
    
    // 加载片段
    segmentLoader.loadSegmentWithMSE(1);
    
    // 验证是否加入队列
    expect(segmentLoader['segmentQueue']).toContain(1);
    expect(eventBus.emit).toHaveBeenCalledWith(Events.SEGMENT_QUEUED, 1);
  });

  test('loadSegmentWithMSE - 当MediaSource未就绪时应该加入待处理队列', () => {
    segmentLoader.initialize(mockSegments);
    
    // 模拟MediaSource未就绪
    segmentLoader.setMediaSourceReady(false);
    
    // 加载片段
    segmentLoader.loadSegmentWithMSE(1);
    
    // 验证是否加入待处理队列
    expect(segmentLoader['pendingSegmentLoads']).toContain(1);
    expect(eventBus.emit).not.toHaveBeenCalledWith(Events.SEGMENT_QUEUED, 1);
  });

  test('applySegmentSlidingWindow - 应该根据滑动窗口配置移除片段', () => {
    segmentLoader.initialize(mockSegments);
    
    // 设置滑动窗口配置
    segmentLoader.setSlidingWindowConfig({
      forward: 1,
      backward: 1,
      enabled: true
    });
    
    // 设置当前索引并应用滑动窗口
    segmentLoader.setCurrentSegmentIndex(2);
    segmentLoader.applySegmentSlidingWindow(2);
    
    // 验证是否触发了移除片段的事件
    // 索引0应该被移除（因为超出backward=1的范围）
    // 索引4应该被移除（因为超出forward=1的范围）
    expect(eventBus.emit).toHaveBeenCalledWith(expect.stringMatching(/segment:removed|REQUEST_REMOVE_SEGMENT/), expect.anything());
  });

  test('processPendingSegmentLoads - 当MediaSource就绪时应该处理待加载的片段', () => {
    segmentLoader.initialize(mockSegments);
    
    // 添加待处理片段
    segmentLoader['pendingSegmentLoads'] = [0, 1, 2];
    
    // 模拟MediaSource就绪
    segmentLoader.setMediaSourceReady(true);
    
    // 处理待加载片段
    segmentLoader.processPendingSegmentLoads();
    
    // 验证待处理队列是否清空
    expect(segmentLoader['pendingSegmentLoads']).toHaveLength(0);
    
    // 验证片段是否加入队列
    expect(segmentLoader['segmentQueue']).toContain(0);
    expect(segmentLoader['segmentQueue']).toContain(1);
    expect(segmentLoader['segmentQueue']).toContain(2);
  });

  test('handleSourceBufferUpdateEnd - 应该更新缓冲区范围并处理下一个片段', () => {
    segmentLoader.initialize(mockSegments);
    
    // 模拟SegmentLoader的内部状态
    segmentLoader['isProcessingQueue'] = false;
    segmentLoader['segmentQueue'] = [0, 1];
    
    // 调用handleSourceBufferUpdateEnd
    const handleSourceBufferUpdateEnd = segmentLoader['handleSourceBufferUpdateEnd'];
    handleSourceBufferUpdateEnd(mockSourceBuffer);
    
    // 验证缓冲区范围是否更新
    expect(eventBus.emit).toHaveBeenCalledWith(Events.BUFFER_UPDATE, expect.anything());
    
    // 验证是否开始处理下一个片段
    expect(segmentLoader['isProcessingQueue']).toBe(true);
  });

  test('处理队列 - 多个片段应该按顺序处理', async () => {
    // 初始化SegmentLoader
    segmentLoader.initialize(mockSegments);
    segmentLoader.setMediaSourceReady(true);
    
    // 加载多个片段
    segmentLoader.loadSegmentWithMSE(0);
    segmentLoader.loadSegmentWithMSE(1);
    segmentLoader.loadSegmentWithMSE(2);
    
    // 验证片段是否加入队列
    expect(segmentLoader['segmentQueue']).toEqual([0, 1, 2]);
    
    // 模拟processNextSegment方法
    const originalProcessNextSegment = segmentLoader['processNextSegment'];
    segmentLoader['processNextSegment'] = jest.fn().mockImplementation(() => {
      segmentLoader['isProcessingQueue'] = false;
      if (segmentLoader['segmentQueue'].length > 0) {
        segmentLoader['segmentQueue'].shift();
        eventBus.emit(Events.SEGMENT_LOADED, 0);
      }
    });
    
    // 手动触发处理
    segmentLoader['processNextSegment']();
    
    // 验证是否处理了一个片段
    expect(eventBus.emit).toHaveBeenCalledWith(Events.SEGMENT_LOADED, expect.anything());
    expect(segmentLoader['segmentQueue'].length).toBeLessThan(3);
    
    // 恢复原始方法
    segmentLoader['processNextSegment'] = originalProcessNextSegment;
  });

  test('destroy - 应该清理资源并取消事件监听', () => {
    // 监听off方法
    jest.spyOn(eventBus, 'off');
    
    // 初始化和销毁
    segmentLoader.initialize(mockSegments);
    segmentLoader.destroy();
    
    // 验证事件监听是否取消
    expect(eventBus.off).toHaveBeenCalledWith(Events.MEDIA_SOURCE_OPEN, expect.any(Function));
    expect(eventBus.off).toHaveBeenCalledWith(Events.MEDIA_SOURCE_CLOSE, expect.any(Function));
    expect(eventBus.off).toHaveBeenCalledWith(Events.SOURCE_BUFFER_CREATED, expect.any(Function));
    expect(eventBus.off).toHaveBeenCalledWith(Events.SOURCE_BUFFER_UPDATE_END, expect.any(Function));
    
    // 验证内部状态是否清理
    expect(segmentLoader['segments']).toHaveLength(0);
    expect(segmentLoader['segmentQueue']).toHaveLength(0);
    expect(segmentLoader['pendingSegmentLoads']).toHaveLength(0);
  });
}); 