/**
 * 测试基于事件总线架构的HLS播放器
 * 
 * 此脚本用于测试新重构的基于事件总线模式的HLS播放器架构
 * 执行方式: node debug-test-event-bus.js
 */
const CDP = require('chrome-remote-interface');
const fs = require('fs');
const path = require('path');

// 存储所有控制台日志
const consoleLogs = [];
// 错误日志计数
let errorCount = 0;

// 默认测试页面地址
const DEFAULT_URL = 'http://localhost:8080';
const TEST_TIMEOUT = 60000; // 60 秒测试超时

/**
 * 打开页面并进行测试
 */
async function testEventBusArchitecture() {
  let client;
  
  try {
    // 连接到已运行的 Chrome
    console.log('正在连接到已运行的 Chrome 实例...');
    client = await CDP();
    
    // 获取 CDP 域
    const { Network, Page, Runtime, DOM, Console } = client;
    
    // 设置事件监听
    Network.requestWillBeSent(params => {
      console.log(`请求: ${params.request.url}`);
    });
    
    // 启用必要的 CDP 功能
    await Promise.all([
      Network.enable(),
      Page.enable(),
      Runtime.enable(),
      DOM.enable(),
      Console.enable()
    ]);
    
    // 监听控制台消息
    Console.messageAdded(({ message }) => {
      const logMessage = `[${message.level}] ${message.text}`;
      consoleLogs.push(logMessage);
      console.log(logMessage);
      
      if (message.level === 'error') {
        errorCount++;
      }
      
      // 如果日志包含 EventBus 相关信息，特别标记出来
      if (message.text.includes('EventBus') || message.text.includes('事件总线')) {
        console.log(`[EventBus 监测] ${message.text}`);
      }
    });
    
    // 导航到测试页面
    console.log(`导航到测试页面: ${DEFAULT_URL}`);
    await Page.navigate({ url: DEFAULT_URL });
    
    // 等待页面加载完成
    await Page.loadEventFired();
    console.log('页面已加载');
    
    // 等待 DOM 内容加载完成
    await Page.domContentEventFired();
    
    // 点击"加载M3U8"按钮，参考 debug-click-m3u8.js 的实现
    console.log('尝试找到并点击"加载M3U8"按钮...');
    const clickResult = await Runtime.evaluate({
      expression: `
        (function() {
          // 尝试多种方式定位按钮
          let button = document.querySelector('#load-m3u8-btn');
          if (!button) {
            // 尝试通过文本内容查找
            let buttons = Array.from(document.querySelectorAll('button'));
            button = buttons.find(b => b.textContent.includes('加载M3U8') || b.textContent.includes('加载 M3U8'));
          }
          if (!button) {
            // 尝试通过类查找
            button = document.querySelector('.n-button');
          }
          
          if (button) {
            console.log('找到加载按钮，准备点击');
            button.click();
            return { success: true, message: '按钮点击成功' };
          } else {
            console.error('未找到加载M3U8按钮');
            return { success: false, message: '未找到加载M3U8按钮' };
          }
        })()
      `
    });
    
    console.log('点击结果:', clickResult.result.value);
    
    // 等待一段时间，让视频加载
    console.log('等待视频加载 (10 秒)...');
    await new Promise(resolve => setTimeout(resolve, 10000));
    
    // 检查视频元素状态
    const videoState = await Runtime.evaluate({
      expression: `
        (function() {
          const video = document.querySelector('video');
          if (!video) return { error: '未找到视频元素' };
          
          const bufferRanges = [];
          for (let i = 0; i < video.buffered.length; i++) {
            bufferRanges.push({
              start: video.buffered.start(i),
              end: video.buffered.end(i)
            });
          }

          return {
            src: video.src,
            currentSrc: video.currentSrc,
            readyState: video.readyState,
            paused: video.paused,
            error: video.error ? video.error.message : null,
            currentTime: video.currentTime,
            duration: video.duration || 0,
            hasMediaSource: video.src.startsWith('blob:'),
            bufferRanges
          };
        })()
      `
    });
    
    console.log('视频元素状态:', videoState.result.value);
    
    // 检查 EventBus 相关状态
    const eventBusState = await Runtime.evaluate({
      expression: `
        (function() {
          // 尝试检查是否存在EventBus全局变量或调试信息
          const windowHasEventBusDebug = typeof window.eventBusDebug !== 'undefined';
          
          // 搜集控制台中的EventBus相关日志
          const eventBusLogs = window.eventBusLogs || [];
          
          // 检查应用内部可能暴露的EventBus状态
          let eventBusStatus = "未知";
          if (window.jessibuca && window.jessibuca.eventBus) {
            eventBusStatus = "可用";
          }
          
          return {
            eventBusExposed: windowHasEventBusDebug,
            eventBusStatus,
            eventBusLogsCount: eventBusLogs.length
          };
        })()
      `
    });
    
    console.log('EventBus状态:', eventBusState.result.value);
    
    // 尝试点击播放按钮
    console.log('尝试点击播放按钮...');
    const playResult = await Runtime.evaluate({
      expression: `
        (function() {
          // 找到播放按钮
          const buttons = Array.from(document.querySelectorAll('button'));
          const playButton = buttons.find(b => 
            b.textContent.includes('播放') || 
            b.textContent.includes('暂停')
          );
          
          if (playButton) {
            console.log('找到播放按钮，准备点击');
            playButton.click();
            return { success: true, message: '播放按钮点击成功' };
          } else {
            console.error('未找到播放按钮');
            return { success: false, message: '未找到播放按钮' };
          }
        })()
      `
    });
    
    console.log('播放按钮点击结果:', playResult.result.value);
    
    // 再等待一段时间，让视频开始播放
    console.log('等待视频播放 (5 秒)...');
    await new Promise(resolve => setTimeout(resolve, 5000));
    
    // 再次检查视频状态
    const videoPlayingState = await Runtime.evaluate({
      expression: `
        (function() {
          const video = document.querySelector('video');
          if (!video) return { error: '未找到视频元素' };
          
          // 获取当前缓冲区范围
          const bufferRanges = [];
          for (let i = 0; i < video.buffered.length; i++) {
            bufferRanges.push({
              start: video.buffered.start(i),
              end: video.buffered.end(i)
            });
          }
          
          return {
            paused: video.paused,
            currentTime: video.currentTime,
            playing: !video.paused,
            hasTimeAdvanced: video.currentTime > 0.1,
            bufferRanges,
            readyState: video.readyState,
            networkState: video.networkState,
            videoWidth: video.videoWidth,
            videoHeight: video.videoHeight
          };
        })()
      `
    });
    
    console.log('视频播放状态:', videoPlayingState.result.value);
    
    // 检查时间线组件状态
    const timelineState = await Runtime.evaluate({
      expression: `
        (function() {
          // 尝试获取时间线组件状态
          const timeline = document.querySelector('.segment-visualizer');
          if (!timeline) return { error: '未找到时间线组件' };
          
          // 获取时间线指针位置
          const pointer = document.querySelector('.segment-visualizer .current-time-indicator');
          const pointerStyle = pointer ? getComputedStyle(pointer) : null;
          
          return {
            found: true,
            pointerExists: !!pointer,
            pointerLeft: pointerStyle ? pointerStyle.left : null,
            pointerPosition: pointer ? pointer.style.left : null,
            containerWidth: timeline.offsetWidth
          };
        })()
      `
    });
    
    console.log('时间线组件状态:', timelineState.result.value);
    
    // 截图保存
    const { data } = await Page.captureScreenshot();
    const screenshotPath = path.join(__dirname, 'event-bus-test-result.png');
    fs.writeFileSync(screenshotPath, Buffer.from(data, 'base64'));
    console.log(`截图已保存到: ${screenshotPath}`);
    
    // 打印测试摘要
    console.log('测试完成!');
    console.log(`收集到 ${consoleLogs.length} 条日志，其中 ${errorCount} 条错误`);
    
    // 最后的检查 - 视频是否正在播放和时间是否在增加
    if (videoPlayingState.result.value.playing) {
      console.log('✅ 视频成功播放');
    } else {
      console.log('❌ 视频未播放');
    }
    
    if (videoPlayingState.result.value.hasTimeAdvanced) {
      console.log('✅ 视频时间正在前进');
    } else {
      console.log('❌ 视频时间未前进');
    }
    
    // 检查时间线指针是否移动
    if (timelineState.result.value.found && timelineState.result.value.pointerLeft) {
      console.log('✅ 时间线指针存在');
      if (parseFloat(timelineState.result.value.pointerLeft) > 0) {
        console.log('✅ 时间线指针已经移动');
      } else {
        console.log('❌ 时间线指针未移动');
      }
    } else {
      console.log('❌ 无法检测时间线指针状态');
    }
    
  } catch (error) {
    console.error('测试过程中发生错误:', error);
  } finally {
    // 清理资源
    if (client) {
      await client.close();
    }
  }
}

// 执行测试，设置整体超时
console.log('开始测试事件总线架构的HLS播放器...');
const testTimeout = setTimeout(() => {
  console.error('测试超时!');
  process.exit(1);
}, TEST_TIMEOUT);

testEventBusArchitecture()
  .then(() => {
    clearTimeout(testTimeout);
    process.exit(0);
  })
  .catch(error => {
    console.error('测试失败:', error);
    clearTimeout(testTimeout);
    process.exit(1);
  }); 