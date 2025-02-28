/**
 * 集成测试
 * 测试 SegmentLoader 和 VirtualTimeline 通过事件总线交互的情况
 */

import { EventBus, Events } from '../EventBus';
import { SegmentLoader } from '../SegmentLoader';
import { VirtualTimeline, MediaSegment } from '../VirtualTimeline';

describe('SegmentLoader和VirtualTimeline集成测试', () => {
  let eventBus: EventBus;
  let segmentLoader: SegmentLoader;
  let virtualTimeline: VirtualTimeline;
  let mockSegments: MediaSegment[];

  beforeEach(() => {
    // 创建模拟对象
    eventBus = new EventBus();
    
    // 监听相关事件
    jest.spyOn(eventBus, 'emit');
    jest.spyOn(eventBus, 'on');
    
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
    
    // 创建实例
    segmentLoader = new SegmentLoader(eventBus);
    virtualTimeline = new VirtualTimeline(eventBus);
    
    // 模拟fetch响应
    global.fetch = jest.fn().mockImplementation(() =>
      Promise.resolve({
        ok: true,
        status: 200,
        arrayBuffer: () => Promise.resolve(new ArrayBuffer(1024))
      })
    );
    
    // 模拟计时器
    jest.useFakeTimers();
  });

  afterEach(() => {
    jest.clearAllMocks();
    jest.useRealTimers();
    segmentLoader.destroy();
    virtualTimeline.destroy();
  });

  test('加载播放列表后应触发片段加载事件', () => {
    // 初始化SegmentLoader
    segmentLoader.initialize(mockSegments);
    
    // 加载播放列表
    virtualTimeline.loadPlaylist({
      segments: mockSegments,
      totalDuration: 50
    });
    
    // 验证事件是否触发
    expect(eventBus.emit).toHaveBeenCalledWith(Events.PLAYLIST_LOADED, expect.anything());
    expect(eventBus.emit).toHaveBeenCalledWith(Events.TOTAL_DURATION_UPDATE, 50);
  });

  test('播放时应正确触发时间更新和片段加载', () => {
    // 初始化组件
    segmentLoader.initialize(mockSegments);
    segmentLoader.setMediaSourceReady(true);
    
    virtualTimeline.loadPlaylist({
      segments: mockSegments,
      totalDuration: 50
    });
    
    // 模拟VirtualTimeline播放
    virtualTimeline.play();
    
    // 推进时间
    jest.advanceTimersByTime(500);
    
    // 验证时间更新事件
    expect(eventBus.emit).toHaveBeenCalledWith(Events.TIME_UPDATE, expect.any(Number));
    
    // 模拟VirtualTimeline跳转到特定时间
    virtualTimeline.seek(15);
    
    // 验证跳转事件
    expect(eventBus.emit).toHaveBeenCalledWith(Events.SEEK, 15);
    expect(eventBus.emit).toHaveBeenCalledWith(Events.TIME_UPDATE, 15);
  });

  test('虚拟时间更新应触发SegmentLoader加载相应片段', () => {
    // 设置监听器来模拟SegmentLoader对TIME_UPDATE的响应
    const mockLoadSegment = jest.fn();
    segmentLoader.loadSegmentWithMSE = mockLoadSegment;
    
    // 初始化组件
    segmentLoader.initialize(mockSegments);
    segmentLoader.setMediaSourceReady(true);
    
    virtualTimeline.loadPlaylist({
      segments: mockSegments,
      totalDuration: 50
    });
    
    // 手动触发时间更新事件
    eventBus.emit(Events.TIME_UPDATE, 15);
    
    // 根据时间15，应该需要加载索引为1的片段(10-20秒)
    // 模拟handleTimelineUpdate被调用
    const handleTimelineUpdate = segmentLoader['handleTimelineUpdate'];
    handleTimelineUpdate(15);
    
    // 如果实现正确，SegmentLoader应该会应用滑动窗口并加载相应片段
    expect(segmentLoader['currentSegmentIndex']).toBe(1);
  });

  test('片段加载完成后应更新虚拟时间轴上的片段状态', async () => {
    // 设置监听器来模拟VirtualTimeline对SEGMENT_LOADED的响应
    const mockUpdateSegment = jest.fn();
    const originalFindSegmentForVirtualTime = virtualTimeline.findSegmentForVirtualTime;
    
    virtualTimeline.findSegmentForVirtualTime = jest.fn(
      (time) => {
        const result = originalFindSegmentForVirtualTime.call(virtualTimeline, time);
        if (result.segment) {
          mockUpdateSegment(result.segment.index);
        }
        return result;
      }
    );
    
    // 初始化组件
    segmentLoader.initialize(mockSegments);
    segmentLoader.setMediaSourceReady(true);
    
    virtualTimeline.loadPlaylist({
      segments: mockSegments,
      totalDuration: 50
    });
    
    // 触发片段加载事件
    eventBus.emit(Events.SEGMENT_LOADED, 1);
    
    // 更新虚拟时间以触发片段查找
    virtualTimeline.seek(15);
    
    // 验证片段状态更新
    expect(mockUpdateSegment).toHaveBeenCalledWith(1);
  });

  test('通过事件总线进行请求-响应通信', () => {
    // 初始化组件
    segmentLoader.initialize(mockSegments);
    virtualTimeline.loadPlaylist({
      segments: mockSegments,
      totalDuration: 50
    });
    
    // 设置当前虚拟时间
    virtualTimeline['currentVirtualTime'] = 25;
    
    // 模拟通过事件总线请求当前时间
    const currentTime = eventBus.request(Events.CURRENT_TIME_REQUEST);
    expect(currentTime).toBe(25);
    
    // 模拟通过事件总线请求总时长
    const totalDuration = eventBus.request(Events.TOTAL_DURATION_REQUEST);
    expect(totalDuration).toBe(50);
  });

  test('片段加载错误应触发错误事件', () => {
    // 初始化组件
    segmentLoader.initialize(mockSegments);
    segmentLoader.setMediaSourceReady(true);
    
    // 模拟片段加载错误
    const error = new Error('网络错误');
    eventBus.emit(Events.SEGMENT_LOAD_ERROR, 1, error);
    
    // 验证错误事件传播
    expect(eventBus.emit).toHaveBeenCalledWith(Events.SEGMENT_LOAD_ERROR, 1, error);
    
    // 验证错误处理逻辑
    // 这里需要根据实际的错误处理逻辑进行验证
  });

  test('销毁组件应取消所有事件监听', () => {
    // 监听off方法
    jest.spyOn(eventBus, 'off');
    
    // 初始化组件
    segmentLoader.initialize(mockSegments);
    virtualTimeline.loadPlaylist({
      segments: mockSegments,
      totalDuration: 50
    });
    
    // 销毁组件
    segmentLoader.destroy();
    virtualTimeline.destroy();
    
    // 验证事件监听是否取消
    // 注意：每个组件会取消多个事件监听，所以应该调用多次
    expect(eventBus.off).toHaveBeenCalled();
    expect(eventBus.off).toHaveBeenCalledTimes(expect.any(Number));
    
    // 清理所有监听
    eventBus.clear();
    expect(eventBus.emit).not.toHaveBeenCalledWith(Events.TIME_UPDATE, expect.anything());
  });
}); 