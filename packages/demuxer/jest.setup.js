/**
 * Jest 设置文件
 * 用于配置测试环境和全局模拟
 */

// 添加自定义匹配器
expect.extend({
  toBeWithinRange(received, floor, ceiling) {
    const pass = received >= floor && received <= ceiling;
    if (pass) {
      return {
        message: () => `expected ${received} not to be within range ${floor} - ${ceiling}`,
        pass: true,
      };
    } else {
      return {
        message: () => `expected ${received} to be within range ${floor} - ${ceiling}`,
        pass: false,
      };
    }
  },
});

// 模拟 MediaSource API
class MockMediaSource {
  constructor() {
    this.readyState = 'closed';
    this.sourceBuffers = {
      length: 0,
      item: () => null,
    };
    this.activeSourceBuffers = {
      length: 0,
      item: () => null,
    };
    this.duration = 0;
  }

  addSourceBuffer() {
    return {
      mode: 'segments',
      updating: false,
      buffered: {
        length: 0,
        start: () => 0,
        end: () => 0,
      },
      appendBuffer: jest.fn(),
      remove: jest.fn(),
      addEventListener: jest.fn(),
      removeEventListener: jest.fn(),
    };
  }

  endOfStream() {}
  removeSourceBuffer() {}
  clearLiveSeekableRange() {}
  setLiveSeekableRange() {}
  addEventListener() {}
  removeEventListener() {}
}

// 模拟 URL API
global.URL = {
  createObjectURL: jest.fn(() => 'blob:mock-url'),
  revokeObjectURL: jest.fn(),
};

// 模拟 MediaSource 全局对象
global.MediaSource = MockMediaSource;
global.MediaSource.isTypeSupported = jest.fn(() => true);

// 模拟 Fetch API
global.fetch = jest.fn().mockImplementation(() => 
  Promise.resolve({
    ok: true,
    status: 200,
    statusText: 'OK',
    headers: new Headers(),
    json: () => Promise.resolve({}),
    text: () => Promise.resolve(''),
    arrayBuffer: () => Promise.resolve(new ArrayBuffer(0)),
  })
); 