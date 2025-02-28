/**
 * Chrome DevTools Protocol HLS测试脚本
 * 
 * 这个脚本连接到已经开启了DevTools Protocol的Chrome浏览器
 * 用于测试MSE功能和HLS播放能力
 */

const CDP = require('chrome-remote-interface');
const path = require('path');
const fs = require('fs');

// CDP连接配置
const CDP_PORT = 9222; // 默认CDP端口
const TIMEOUT = 30000; // 30秒超时

/**
 * 启动脚本说明（需要先手动执行）:
 * 
 * Windows:
 * "C:\Program Files\Google\Chrome\Application\chrome.exe" --remote-debugging-port=9222 --autoplay-policy=no-user-gesture-required
 * 
 * macOS:
 * /Applications/Google\ Chrome.app/Contents/MacOS/Google\ Chrome --remote-debugging-port=9222 --autoplay-policy=no-user-gesture-required
 * 
 * Linux:
 * google-chrome --remote-debugging-port=9222 --autoplay-policy=no-user-gesture-required
 */

async function runTest() {
  console.log('正在连接到已运行的Chrome实例...');
  let client;

  try {
    // 连接到Chrome
    client = await CDP({
      port: CDP_PORT,
      host: 'localhost'
    });

    // 解构需要用到的CDP域
    const { Network, Page, Runtime, Console, Media } = client;

    // 设置事件监听
    await Promise.all([
      Network.enable(),
      Page.enable(),
      Console.enable(),
      // 如果支持Media域，则启用它（不是所有Chrome版本都支持）
      Media && Media.enable().catch(() => console.log('Media域不可用，继续执行...')),
      Runtime.enable()
    ]);

    // 监听控制台消息
    Console.messageAdded((params) => {
      const { level, text, url } = params.message;
      console.log(`[浏览器控制台 ${level}]: ${text} ${url ? `(${url})` : ''}`);
    });

    // 监听页面错误
    Runtime.exceptionThrown((params) => {
      const { exceptionDetails } = params;
      console.error('页面错误:', exceptionDetails.text, exceptionDetails.exception && exceptionDetails.exception.description);
    });

    // 导航到测试页面（使用本地服务器）
    console.log('导航到本地测试页面...');
    await Page.navigate({ url: 'http://localhost:3001/debug.html' });

    // 等待页面加载完成
    await Page.loadEventFired();
    console.log('页面加载完成');

    // 检查媒体功能支持情况
    console.log('检查媒体支持情况...');
    const mediaSupportResult = await Runtime.evaluate({
      expression: `
        (function checkMediaSupport() {
          const results = {
            mediaSource: typeof MediaSource !== 'undefined',
            codecs: {}
          };
          
          if (results.mediaSource) {
            // 测试关键编解码器
            const codecs = [
              'video/mp4; codecs="avc1.64001f"',
              'video/mp4; codecs="avc1.42E01E"',
              'video/mp4; codecs="avc1.4D401E"',
              'video/mp4',
              'video/webm; codecs="vp8"',
              'video/webm; codecs="vp9"'
            ];
            
            codecs.forEach(codec => {
              results.codecs[codec] = MediaSource.isTypeSupported(codec);
            });
          }
          
          return results;
        })()
      `,
      returnByValue: true
    });

    const mediaSupport = mediaSupportResult.result.value;
    console.log('媒体支持情况:', JSON.stringify(mediaSupport, null, 2));

    // 初始化播放器
    console.log('初始化HLS播放器...');
    await Runtime.evaluate({
      expression: `
        (function initPlayer() {
          // 如果页面上有初始化按钮，点击它
          const initButton = document.querySelector('button:nth-child(1)');
          if (initButton) {
            console.log('点击初始化按钮...');
            initButton.click();
          } else {
            console.log('未找到初始化按钮，假设播放器已自动初始化');
          }
        })()
      `
    });

    // 等待初始化
    await new Promise(resolve => setTimeout(resolve, 3000));
    console.log('等待初始化完成');

    // 尝试播放
    console.log('尝试播放...');
    const playResult = await Runtime.evaluate({
      expression: `
        (function playVideo() {
          // 如果页面上有播放按钮，点击它
          const playButton = document.querySelector('button:nth-child(2)');
          if (playButton) {
            console.log('点击播放按钮...');
            playButton.click();
            return true;
          } else {
            // 尝试直接调用播放功能
            if (window.player && typeof window.player.play === 'function') {
              console.log('调用player.play()...');
              window.player.play();
              return true;
            }
            
            // 尝试找到并播放视频元素
            const videoElement = document.querySelector('video');
            if (videoElement) {
              console.log('直接调用video.play()...');
              videoElement.play().then(() => {
                console.log('播放成功启动');
              }).catch(err => {
                console.error('播放失败:', err.message);
              });
              return true;
            }
            
            return false;
          }
        })()
      `,
      returnByValue: true
    });

    if (!playResult.result.value) {
      console.log('未找到播放控件，请检查页面结构');
    }

    // 等待播放缓冲
    await new Promise(resolve => setTimeout(resolve, 5000));

    // 检查播放状态
    const playbackStatus = await Runtime.evaluate({
      expression: `
        (function checkPlayback() {
          const videoElement = document.querySelector('video');
          if (!videoElement) return { error: 'No video element found' };
          
          // 收集了视频播放的详细状态信息
          return {
            readyState: videoElement.readyState,
            networkState: videoElement.networkState,
            currentTime: videoElement.currentTime,
            duration: videoElement.duration,
            paused: videoElement.paused,
            ended: videoElement.ended,
            error: videoElement.error ? {
              code: videoElement.error.code,
              message: videoElement.error.message
            } : null,
            buffered: Array.from({ length: videoElement.buffered.length }, 
              (_, i) => ({
                start: videoElement.buffered.start(i),
                end: videoElement.buffered.end(i)
              })
            ),
            demuxer: window.player && window.player.demuxer ? {
              totalDuration: window.player.demuxer.getTotalDuration(),
              currentTime: window.player.demuxer.getCurrentTime(),
              sourceBuffer: window.player.demuxer.sourceBuffer ? 'Available' : 'Not available',
              codec: window.player.demuxer.codec
            } : 'Not available'
          };
        })()
      `,
      returnByValue: true
    });

    console.log('播放状态:', JSON.stringify(playbackStatus.result.value, null, 2));

    // 尝试跳转
    console.log('尝试跳转到10秒位置...');
    await Runtime.evaluate({
      expression: `
        (function seekVideo() {
          // 如果有跳转按钮，点击它
          const seekButton = document.querySelector('button:nth-child(4)'); // 通常第4个按钮是跳转
          if (seekButton) {
            console.log('点击跳转按钮...');
            seekButton.click();
            return true;
          } else {
            // 尝试直接调用seek功能
            if (window.player && typeof window.player.seek === 'function') {
              console.log('调用player.seek(10)...');
              window.player.seek(10);
              return true;
            }
            
            // 尝试直接设置视频时间
            const videoElement = document.querySelector('video');
            if (videoElement) {
              console.log('直接设置video.currentTime = 10...');
              videoElement.currentTime = 10;
              return true;
            }
            
            return false;
          }
        })()
      `,
      returnByValue: true
    });

    // 等待跳转完成
    await new Promise(resolve => setTimeout(resolve, 5000));

    // 检查跳转后状态
    const seekStatus = await Runtime.evaluate({
      expression: `
        (function checkSeekStatus() {
          const videoElement = document.querySelector('video');
          if (!videoElement) return { error: 'No video element found' };
          
          return {
            currentTime: videoElement.currentTime,
            paused: videoElement.paused,
            seeking: videoElement.seeking,
            readyState: videoElement.readyState,
            buffered: Array.from({ length: videoElement.buffered.length }, 
              (_, i) => ({
                start: videoElement.buffered.start(i),
                end: videoElement.buffered.end(i)
              })
            )
          };
        })()
      `,
      returnByValue: true
    });

    console.log('跳转后状态:', JSON.stringify(seekStatus.result.value, null, 2));

    // 获取调试信息
    const debugInfo = await Runtime.evaluate({
      expression: `
        (function getDebugInfo() {
          // 收集MSE和播放器相关的调试信息
          
          // 检查MediaSource状态
          let mediaSourceStatus = 'Not available';
          if (window.player && window.player.demuxer && window.player.demuxer.mediaSource) {
            const ms = window.player.demuxer.mediaSource;
            mediaSourceStatus = {
              readyState: ms.readyState,
              duration: ms.duration
            };
          }
          
          // 收集整体调试信息
          return {
            mediaSource: mediaSourceStatus,
            hlsPlayerVersion: window.player && window.player.version,
            browserCodecs: {
              h264: MediaSource.isTypeSupported('video/mp4; codecs="avc1.42E01E"'),
              h264High: MediaSource.isTypeSupported('video/mp4; codecs="avc1.64001F"'),
              vp8: MediaSource.isTypeSupported('video/webm; codecs="vp8"'),
              vp9: MediaSource.isTypeSupported('video/webm; codecs="vp9"')
            },
            errors: window.__TEST_ERRORS || []
          };
        })()
      `,
      returnByValue: true
    });

    console.log('调试信息:', JSON.stringify(debugInfo.result.value, null, 2));

    // 增加等待时间，以便观察浏览器中的内容
    console.log('测试完成，等待30秒以便观察浏览器中的内容...');
    await new Promise(resolve => setTimeout(resolve, 30000));

  } catch (err) {
    console.error('测试过程中出错:', err);
  } finally {
    // 关闭连接
    if (client) {
      await client.close();
    }
    console.log('CDP连接已关闭');
  }
}

// 运行测试
console.log('========== CDP HLS测试开始 ==========');
console.log(`请确保Chrome已经运行并开启了远程调试端口 ${CDP_PORT}`);
console.log('启动Chrome的命令示例:');
console.log('Windows: "C:\\Program Files\\Google\\Chrome\\Application\\chrome.exe" --remote-debugging-port=9222 --autoplay-policy=no-user-gesture-required');
console.log('macOS: /Applications/Google\\ Chrome.app/Contents/MacOS/Google\\ Chrome --remote-debugging-port=9222 --autoplay-policy=no-user-gesture-required');
console.log('Linux: google-chrome --remote-debugging-port=9222 --autoplay-policy=no-user-gesture-required');
console.log('======================================');

runTest().catch(err => {
  console.error('测试运行失败:', err);
  process.exit(1);
}); 