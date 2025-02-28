/**
 * EventBus 单元测试
 * 测试事件总线的核心功能和事件常量的一致性
 */

import { EventBus, Events } from '../EventBus';

describe('EventBus 基础功能测试', () => {
  let eventBus: EventBus;

  beforeEach(() => {
    eventBus = new EventBus();
  });

  test('on/emit - 事件订阅与触发', () => {
    const mockFn = jest.fn();
    eventBus.on('test', mockFn);
    
    eventBus.emit('test', 'arg1', 'arg2');
    expect(mockFn).toHaveBeenCalledTimes(1);
    expect(mockFn).toHaveBeenCalledWith('arg1', 'arg2');
    
    eventBus.emit('test', 'arg3');
    expect(mockFn).toHaveBeenCalledTimes(2);
    expect(mockFn).toHaveBeenLastCalledWith('arg3');
  });

  test('off - 取消事件订阅', () => {
    const mockFn = jest.fn();
    eventBus.on('test', mockFn);
    
    eventBus.emit('test');
    expect(mockFn).toHaveBeenCalledTimes(1);
    
    eventBus.off('test', mockFn);
    eventBus.emit('test');
    expect(mockFn).toHaveBeenCalledTimes(1); // 不应该再次被调用
  });

  test('request - 返回监听器结果', () => {
    const mockFn = jest.fn().mockReturnValue('result');
    eventBus.on('request', mockFn);
    
    const result = eventBus.request('request', 'arg');
    expect(result).toBe('result');
    expect(mockFn).toHaveBeenCalledWith('arg');
  });

  test('request - 没有监听器时返回null', () => {
    const result = eventBus.request('nonexistent');
    expect(result).toBeNull();
  });

  test('clear - 清除所有监听器', () => {
    const mockFn1 = jest.fn();
    const mockFn2 = jest.fn();
    
    eventBus.on('event1', mockFn1);
    eventBus.on('event2', mockFn2);
    
    eventBus.emit('event1');
    eventBus.emit('event2');
    
    expect(mockFn1).toHaveBeenCalledTimes(1);
    expect(mockFn2).toHaveBeenCalledTimes(1);
    
    eventBus.clear();
    
    eventBus.emit('event1');
    eventBus.emit('event2');
    
    expect(mockFn1).toHaveBeenCalledTimes(1); // 不应该增加
    expect(mockFn2).toHaveBeenCalledTimes(1); // 不应该增加
  });

  test('多个监听器处理同一事件', () => {
    const mockFn1 = jest.fn();
    const mockFn2 = jest.fn();
    
    eventBus.on('event', mockFn1);
    eventBus.on('event', mockFn2);
    
    eventBus.emit('event', 'data');
    
    expect(mockFn1).toHaveBeenCalledWith('data');
    expect(mockFn2).toHaveBeenCalledWith('data');
  });
});

describe('Events 常量测试', () => {
  test('确保关键事件常量已定义', () => {
    // 检查关键事件常量是否存在
    expect(Events.ERROR).toBeDefined();
    expect(Events.LOG).toBeDefined();
    expect(Events.MEDIA_SOURCE_OPEN).toBeDefined();
    expect(Events.VIDEO_PLAY).toBeDefined();
    expect(Events.SEGMENT_LOADED).toBeDefined();
    expect(Events.PLAYLIST_LOADED).toBeDefined();
    expect(Events.APPEND_BUFFER).toBeDefined();
    expect(Events.APPEND_INIT_SEGMENT).toBeDefined();
  });

  test('确保需要的事件常量包含相应的字符串模式', () => {
    // 检查常量值是否遵循一致的命名约定
    expect(Events.MEDIA_SOURCE_OPEN).toContain('mediaSource');
    expect(Events.VIDEO_PLAY).toContain('video');
    expect(Events.SEGMENT_LOADED).toContain('segment');
    expect(Events.BUFFER_UPDATE).toContain('buffer');
  });
}); 