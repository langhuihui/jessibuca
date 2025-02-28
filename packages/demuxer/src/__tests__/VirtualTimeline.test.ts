/**
 * VirtualTimeline 单元测试
 * 测试时间轴管理、片段查找和事件触发等核心功能
 */

import { EventBus, Events } from '../EventBus';
import { VirtualTimeline, MediaSegment, PlaylistInfo, BufferRange } from '../VirtualTimeline';

describe('VirtualTimeline 基础功能测试', () => {
  let eventBus: EventBus;
  let virtualTimeline: VirtualTimeline;
  let mockSegments: MediaSegment[];
  let mockPlaylistInfo: PlaylistInfo;

  beforeEach(() => {
    // 创建模拟对象
    eventBus = new EventBus();
    
    // 监听相关事件
    jest.spyOn(eventBus, 'emit');
    jest.spyOn(eventBus, 'on');
    jest.spyOn(eventBus, 'request');
    
    // 创建模拟片段
    mockSegments = Array.from({ length: 5 }, (_, i) => ({
      index: i,
      url: `http://example.com/segment_${i}.mp4`,
      duration: 10,
      virtualStartTime: i * 10,
      virtualEndTime: (i + 1) * 10,
      physicalStartTime: i * 10 + 1000, // 物理时间与虚拟时间有偏移
      physicalEndTime: (i + 1) * 10 + 1000,
      physicalTime: new Date(Date.now() + i * 10000),
      codec: 'avc1.64001E',
      isLoaded: false,
      isLoading: false,
      isBuffered: false
    }));
    
    // 创建模拟播放列表信息
    mockPlaylistInfo = {
      segments: mockSegments,
      totalDuration: 50 // 5个片段，每个10秒
    };
    
    // 创建VirtualTimeline实例
    virtualTimeline = new VirtualTimeline(eventBus);
    
    // 模拟计时器
    jest.useFakeTimers();
  });

  afterEach(() => {
    jest.clearAllMocks();
    jest.useRealTimers();
    virtualTimeline.destroy();
  });

  test('loadPlaylist - 应该正确加载播放列表并触发事件', () => {
    virtualTimeline.loadPlaylist(mockPlaylistInfo);
    
    // 验证事件是否触发
    expect(eventBus.emit).toHaveBeenCalledWith(Events.PLAYLIST_LOADED, mockPlaylistInfo);
    expect(eventBus.emit).toHaveBeenCalledWith(Events.TOTAL_DURATION_UPDATE, 50);
    
    // 验证内部状态
    expect(virtualTimeline['segments']).toEqual(mockSegments);
    expect(virtualTimeline['virtualTotalDuration']).toBe(50);
  });

  test('createPlaylistFromM3U8 - 应该正确解析M3U8内容', () => {
    const m3u8Content = `#EXTM3U
#EXT-X-VERSION:3
#EXT-X-TARGETDURATION:10
#EXT-X-MEDIA-SEQUENCE:0
#EXTINF:10.0,2023-10-01T12:00:00Z,avc1.64001E
segment_0.mp4
#EXTINF:10.0,2023-10-01T12:00:10Z,avc1.64001E
segment_1.mp4
#EXTINF:10.0,2023-10-01T12:00:20Z,avc1.64001E
segment_2.mp4
#EXT-X-ENDLIST`;
    
    const baseUrl = 'http://example.com/';
    const playlist = virtualTimeline.createPlaylistFromM3U8(m3u8Content, baseUrl);
    
    expect(playlist.segments.length).toBe(3);
    expect(playlist.totalDuration).toBe(30);
    
    // 验证第一个片段
    const firstSegment = playlist.segments[0];
    expect(firstSegment.url).toBe('http://example.com/segment_0.mp4');
    expect(firstSegment.duration).toBe(10);
    expect(firstSegment.codec).toBe('avc1.64001E');
    expect(firstSegment.virtualStartTime).toBe(0);
    expect(firstSegment.virtualEndTime).toBe(10);
  });

  test('findSegmentForVirtualTime - 应该找到对应时间的片段', () => {
    virtualTimeline.loadPlaylist(mockPlaylistInfo);
    
    // 查找中间时间点的片段
    const result1 = virtualTimeline.findSegmentForVirtualTime(15);
    expect(result1.segment).toBe(mockSegments[1]);
    expect(result1.offsetInSegment).toBe(5); // 15 - 10 = 5
    
    // 查找片段边界的时间点
    const result2 = virtualTimeline.findSegmentForVirtualTime(20);
    expect(result2.segment).toBe(mockSegments[2]);
    expect(result2.offsetInSegment).toBe(0);
    
    // 查找超出范围的时间点
    const result3 = virtualTimeline.findSegmentForVirtualTime(60);
    expect(result3.segment).toBeNull();
  });

  test('时间转换函数 - 应该正确转换虚拟时间和物理时间', () => {
    virtualTimeline.loadPlaylist(mockPlaylistInfo);
    
    // 测试虚拟时间到物理时间的转换
    const physicalTime = virtualTimeline.virtualToPhysicalTime(15);
    expect(physicalTime).toBe(1015); // 15 + 1000 (偏移量)
    
    // 测试物理时间到虚拟时间的转换
    const virtualTime = virtualTimeline.physicalToVirtualTime(1025);
    expect(virtualTime).toBe(25); // 1025 - 1000 (偏移量)
  });

  test('getCurrentVirtualTime 和 getTotalDuration - 应该返回正确的时间信息', () => {
    virtualTimeline.loadPlaylist(mockPlaylistInfo);
    
    // 模拟当前时间
    virtualTimeline['currentVirtualTime'] = 25;
    
    // 测试获取当前虚拟时间
    expect(virtualTimeline.getCurrentVirtualTime()).toBe(25);
    
    // 测试获取总时长
    expect(virtualTimeline.getTotalDuration()).toBe(50);
  });

  test('getProgress - 应该返回正确的播放进度百分比', () => {
    virtualTimeline.loadPlaylist(mockPlaylistInfo);
    
    // 模拟不同的当前时间
    virtualTimeline['currentVirtualTime'] = 0;
    expect(virtualTimeline.getProgress()).toBe(0);
    
    virtualTimeline['currentVirtualTime'] = 25;
    expect(virtualTimeline.getProgress()).toBe(50); // 25/50 * 100 = 50%
    
    virtualTimeline['currentVirtualTime'] = 50;
    expect(virtualTimeline.getProgress()).toBe(100); // 50/50 * 100 = 100%
  });

  test('play 和 pause - 应该正确控制播放状态', () => {
    virtualTimeline.loadPlaylist(mockPlaylistInfo);
    
    // 测试播放
    virtualTimeline.play();
    expect(virtualTimeline['isPlaying']).toBe(true);
    expect(eventBus.emit).toHaveBeenCalledWith(Events.PLAY);
    
    // 推进定时器
    jest.advanceTimersByTime(500);
    expect(virtualTimeline['currentVirtualTime']).toBeGreaterThan(0);
    
    // 测试暂停
    virtualTimeline.pause();
    expect(virtualTimeline['isPlaying']).toBe(false);
    expect(eventBus.emit).toHaveBeenCalledWith(Events.PAUSE);
    
    // 记录当前时间
    const timeAtPause = virtualTimeline['currentVirtualTime'];
    
    // 推进定时器
    jest.advanceTimersByTime(500);
    
    // 暂停后时间不应变化
    expect(virtualTimeline['currentVirtualTime']).toBe(timeAtPause);
  });

  test('seek - 应该正确跳转到指定时间', () => {
    virtualTimeline.loadPlaylist(mockPlaylistInfo);
    
    // 跳转到中间位置
    virtualTimeline.seek(25);
    
    // 验证内部状态和事件
    expect(virtualTimeline['currentVirtualTime']).toBe(25);
    expect(eventBus.emit).toHaveBeenCalledWith(Events.SEEK, 25);
    expect(eventBus.emit).toHaveBeenCalledWith(Events.TIME_UPDATE, 25);
    
    // 跳转到超出范围的位置应该被限制
    virtualTimeline.seek(60);
    expect(virtualTimeline['currentVirtualTime']).toBeLessThanOrEqual(50);
    
    // 跳转到负时间应该被限制
    virtualTimeline.seek(-10);
    expect(virtualTimeline['currentVirtualTime']).toBeGreaterThanOrEqual(0);
  });

  test('setPlaybackRate - 应该正确设置播放速率', () => {
    virtualTimeline.loadPlaylist(mockPlaylistInfo);
    
    // 测试设置不同播放速率
    virtualTimeline.setPlaybackRate(2);
    expect(virtualTimeline['playbackRate']).toBe(2);
    
    virtualTimeline.setPlaybackRate(0.5);
    expect(virtualTimeline['playbackRate']).toBe(0.5);
    
    // 测试无效值
    virtualTimeline.setPlaybackRate(0);
    expect(virtualTimeline['playbackRate']).toBe(0.5); // 不应改变
    
    virtualTimeline.setPlaybackRate(-1);
    expect(virtualTimeline['playbackRate']).toBe(0.5); // 不应改变
  });

  test('handleVideoTimeUpdate - 应该正确处理视频时间更新', () => {
    virtualTimeline.loadPlaylist(mockPlaylistInfo);
    
    // 模拟视频时间更新事件
    const handleVideoTimeUpdate = virtualTimeline['handleVideoTimeUpdate'];
    handleVideoTimeUpdate(15);
    
    // 验证虚拟时间是否更新
    expect(virtualTimeline['currentVirtualTime']).toBe(15);
    expect(eventBus.emit).toHaveBeenCalledWith(Events.TIME_UPDATE, 15);
  });

  test('destroy - 应该正确清理资源', () => {
    // 监听off方法
    jest.spyOn(eventBus, 'off');
    
    virtualTimeline.loadPlaylist(mockPlaylistInfo);
    virtualTimeline.play(); // 启动定时器
    
    // 销毁实例
    virtualTimeline.destroy();
    
    // 验证事件监听是否取消
    expect(eventBus.off).toHaveBeenCalled();
    
    // 验证内部状态是否清理
    expect(virtualTimeline['segments']).toHaveLength(0);
    expect(virtualTimeline['isPlaying']).toBe(false);
    expect(virtualTimeline['playbackTimer']).toBeNull();
  });

  test('emitTimeUpdate - 应该触发时间更新事件', () => {
    virtualTimeline.loadPlaylist(mockPlaylistInfo);
    virtualTimeline['currentVirtualTime'] = 25;
    
    // 调用emitTimeUpdate
    const emitTimeUpdate = virtualTimeline['emitTimeUpdate'];
    emitTimeUpdate();
    
    // 验证事件是否触发
    expect(eventBus.emit).toHaveBeenCalledWith(Events.TIME_UPDATE, 25);
  });

  test('处理响应请求 - 应该返回正确的时间信息', () => {
    virtualTimeline.loadPlaylist(mockPlaylistInfo);
    virtualTimeline['currentVirtualTime'] = 25;
    
    // 测试处理当前时间请求
    const handleCurrentTimeRequest = virtualTimeline['handleCurrentTimeRequest'];
    const currentTime = handleCurrentTimeRequest();
    expect(currentTime).toBe(25);
    
    // 测试处理总时长请求
    const handleTotalDurationRequest = virtualTimeline['handleTotalDurationRequest'];
    const totalDuration = handleTotalDurationRequest();
    expect(totalDuration).toBe(50);
  });
}); 