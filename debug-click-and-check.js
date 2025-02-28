const CDP = require('chrome-remote-interface');
const fs = require('fs');
const path = require('path');

// 存储所有控制台日志
const consoleLogs = [];
// 错误日志计数
let errorCount = 0;

/**
 * 简单的测试脚本：连接到已运行的Chrome，点击加载M3U8按钮，然后检查播放状态
 */
async function testPlayer() {
  let client;
  
  try {
    // 连接到已运行的 Chrome
    console.log('正在连接到已运行的 Chrome 实例...');
    client = await CDP();
    
    // 获取当前选项卡信息
    const { target } = client;
    console.log(`连接到标签页: ${target.url || '未知URL'}`);
    
    // 获取 CDP 域
    const { Network, Page, Runtime, Console } = client;
    
    // 启用必要的 CDP 功能
    await Promise.all([
      Network.enable(),
      Page.enable(),
      Runtime.enable(),
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
    });
    
    // 导航到测试页面
    const testUrl = 'http://localhost:5173/';
    console.log(`导航到测试页面: ${testUrl}`);
    await Page.navigate({ url: testUrl });
    
    // 等待页面加载完成
    await Page.loadEventFired();
    console.log('页面已加载完成');
    
    // 等待一段时间确保页面完全渲染
    await new Promise(resolve => setTimeout(resolve, 2000));
    
    // 截图页面作为初始状态记录
    const { data: initialData } = await Page.captureScreenshot();
    const initialScreenshotPath = path.join(__dirname, 'initial-page.png');
    fs.writeFileSync(initialScreenshotPath, Buffer.from(initialData, 'base64'));
    console.log(`初始页面截图已保存: ${initialScreenshotPath}`);
    
    // 检查页面上是否有视频元素
    console.log('检查页面上是否已存在视频元素...');
    const initialCheck = await Runtime.evaluate({
      expression: `
        (function() {
          return {
            video: document.querySelector('video') !== null,
            loadButton: document.querySelector('#load-m3u8-btn') !== null || 
                        Array.from(document.querySelectorAll('button')).some(b => 
                          b.textContent.includes('加载M3U8') || b.textContent.includes('加载 M3U8')),
            title: document.title,
            url: window.location.href
          };
        })()
      `,
      returnByValue: true
    });
    
    console.log('初始页面状态:', initialCheck.result.value);
    
    // 如果找不到加载按钮，但页面可能已经加载了视频，则跳过点击加载按钮的步骤
    let clickedLoadButton = false;
    
    if (initialCheck.result.value && initialCheck.result.value.loadButton) {
      // 点击"加载M3U8"按钮
      console.log('尝试点击"加载M3U8"按钮...');
      const clickResult = await Runtime.evaluate({
        expression: `
          (function() {
            let button = document.querySelector('#load-m3u8-btn');
            if (!button) {
              let buttons = Array.from(document.querySelectorAll('button'));
              button = buttons.find(b => b.textContent.includes('加载M3U8') || b.textContent.includes('加载 M3U8'));
            }
            
            if (button) {
              console.log('[测试脚本] 找到加载按钮，点击中...');
              button.click();
              return { success: true };
            } else {
              console.error('[测试脚本] 未找到加载M3U8按钮');
              return { success: false };
            }
          })()
        `,
        returnByValue: true
      });
      
      console.log('点击结果:', clickResult.result.value);
      clickedLoadButton = clickResult.result.value && clickResult.result.value.success;
    } else {
      console.log('跳过点击加载按钮，因为页面上没有找到加载按钮或已经有视频元素');
    }
    
    // 等待视频加载 (无论是通过点击按钮还是页面已经有视频)
    console.log('等待视频加载 (10 秒)...');
    await new Promise(resolve => setTimeout(resolve, 10000));
    
    // 查询视频状态
    console.log('检查视频状态...');
    const videoState = await Runtime.evaluate({
      expression: `
        (function() {
          const video = document.querySelector('video');
          if (!video) return { error: '未找到视频元素' };
          
          // 获取缓冲区信息
          const bufferRanges = [];
          try {
            for (let i = 0; i < video.buffered.length; i++) {
              bufferRanges.push({
                start: video.buffered.start(i),
                end: video.buffered.end(i)
              });
            }
          } catch (e) {
            bufferRanges.push({ error: e.toString() });
          }
          
          return {
            found: true,
            src: video.src.substring(0, 40) + '...',
            readyState: video.readyState,
            networkState: video.networkState,
            paused: video.paused,
            hasError: video.error !== null,
            error: video.error ? video.error.message : null,
            currentTime: video.currentTime,
            duration: video.duration || 0,
            bufferRanges,
            videoWidth: video.videoWidth,
            videoHeight: video.videoHeight
          };
        })()
      `,
      returnByValue: true
    });
    
    if (!videoState.result.value || videoState.result.value.error) {
      console.log('未找到视频元素，可能是未加载成功');
      
      // 即使找不到视频元素，也继续检查页面结构
      console.log('检查页面结构...');
      const pageStructure = await Runtime.evaluate({
        expression: `
          (function() {
            try {
              return {
                title: document.title,
                url: window.location.href,
                bodyChildren: Array.from(document.body.children).map(c => c.tagName).join(', '),
                buttons: Array.from(document.querySelectorAll('button')).map(b => b.textContent).join(', '),
                childrenCount: document.body.children.length
              };
            } catch (e) {
              return { error: e.toString() };
            }
          })()
        `,
        returnByValue: true
      });
      
      console.log('页面结构:', pageStructure.result.value);
    } else {
      console.log('视频状态:', videoState.result.value);
      
      // 检查时间线组件状态
      console.log('检查时间线组件状态...');
      const timelineState = await Runtime.evaluate({
        expression: `
          (function() {
            try {
              const timeline = document.querySelector('.segment-visualizer');
              if (!timeline) return { found: false, message: '未找到时间线组件' };
              
              const timeIndicator = document.querySelector('.segment-visualizer .current-time-indicator');
              let indicatorPosition = null;
              
              if (timeIndicator) {
                // 获取计算后的样式
                const computedStyle = window.getComputedStyle(timeIndicator);
                indicatorPosition = {
                  left: computedStyle.left,
                  transform: computedStyle.transform,
                  inlineStyle: timeIndicator.style.left || timeIndicator.style.transform
                };
              }
              
              return {
                found: true,
                indicatorFound: !!timeIndicator,
                indicatorPosition: indicatorPosition,
                segments: document.querySelectorAll('.segment-visualizer .segment').length,
                containerWidth: timeline.offsetWidth
              };
            } catch (e) {
              return { error: e.toString() };
            }
          })()
        `,
        returnByValue: true
      });
      
      console.log('时间线状态:', timelineState.result.value);
      
      // 尝试点击播放按钮
      if (videoState.result.value.paused) {
        console.log('视频当前是暂停状态，尝试点击播放按钮...');
        const playResult = await Runtime.evaluate({
          expression: `
            (function() {
              try {
                const buttons = Array.from(document.querySelectorAll('button'));
                const playButton = buttons.find(b => 
                  b.textContent.includes('播放') || 
                  b.textContent.includes('暂停')
                );
                
                if (playButton) {
                  console.log('[测试脚本] 找到播放按钮，点击中...');
                  playButton.click();
                  return { success: true };
                } else {
                  console.error('[测试脚本] 未找到播放按钮');
                  // 尝试直接调用视频的play()方法
                  const video = document.querySelector('video');
                  if (video) {
                    console.log('[测试脚本] 尝试直接调用视频的play()方法');
                    video.play().catch(e => console.error('[测试脚本] 播放错误:', e));
                    return { success: true, method: 'direct-play' };
                  }
                  return { success: false };
                }
              } catch (e) {
                return { error: e.toString() };
              }
            })()
          `,
          returnByValue: true
        });
        
        console.log('播放结果:', playResult.result.value);
        
        // 等待视频播放开始
        console.log('等待视频播放 (5 秒)...');
        await new Promise(resolve => setTimeout(resolve, 5000));
      } else {
        console.log('视频已经在播放中...');
      }
      
      // 检查播放状态
      console.log('检查最终播放状态...');
      const playingState = await Runtime.evaluate({
        expression: `
          (function() {
            try {
              const video = document.querySelector('video');
              if (!video) return { error: '未找到视频元素' };
              
              return {
                isPlaying: !video.paused,
                currentTime: video.currentTime,
                hasData: video.readyState > 1,
                duration: video.duration || 0,
                bufferStart: video.buffered.length > 0 ? video.buffered.start(0) : -1,
                bufferEnd: video.buffered.length > 0 ? video.buffered.end(0) : -1
              };
            } catch (e) {
              return { error: e.toString() };
            }
          })()
        `,
        returnByValue: true
      });
      
      console.log('最终播放状态:', playingState.result.value);
      
      // 再次检查时间线指示器位置
      if (timelineState.result.value && timelineState.result.value.found && timelineState.result.value.indicatorFound) {
        console.log('再次检查时间线指示器位置...');
        const finalIndicatorState = await Runtime.evaluate({
          expression: `
            (function() {
              try {
                const indicator = document.querySelector('.segment-visualizer .current-time-indicator');
                if (!indicator) return { found: false };
                
                const computedStyle = window.getComputedStyle(indicator);
                
                return {
                  found: true,
                  left: computedStyle.left,
                  transform: computedStyle.transform,
                  offsetLeft: indicator.offsetLeft,
                  width: indicator.offsetWidth
                };
              } catch (e) {
                return { error: e.toString() };
              }
            })()
          `,
          returnByValue: true
        });
        
        console.log('最终时间线指示器位置:', finalIndicatorState.result.value);
      }
    }
    
    // 截图最终状态
    console.log('截图最终页面状态...');
    const { data: finalData } = await Page.captureScreenshot();
    const finalScreenshotPath = path.join(__dirname, 'final-player-state.png');
    fs.writeFileSync(finalScreenshotPath, Buffer.from(finalData, 'base64'));
    console.log(`最终页面截图已保存: ${finalScreenshotPath}`);
    
    // 测试总结
    console.log('\n=== 测试总结 ===');
    console.log(`- 收集到 ${consoleLogs.length} 条日志 (${errorCount} 条错误)`);
    
    if (videoState.result && videoState.result.value) {
      if (videoState.result.value.hasError) {
        console.log('❌ 视频加载出错:', videoState.result.value.error);
      } else if (videoState.result.value.found) {
        console.log('✅ 视频加载成功');
        
        if (videoState.result.value.src && videoState.result.value.src.startsWith('blob:')) {
          console.log('✅ 正确使用MediaSource (Blob URL)');
        } else {
          console.log('❓ 未使用MediaSource Blob URL');
        }
      }
    } else {
      console.log('❌ 无法获取视频状态');
    }
    
    if (clickedLoadButton) {
      console.log('✅ 成功点击加载M3U8按钮');
    }
    
    const hasPlayingResult = typeof playingState !== 'undefined' && playingState && playingState.result && playingState.result.value;
    
    if (hasPlayingResult) {
      if (playingState.result.value.isPlaying) {
        console.log('✅ 视频正在播放');
      } else {
        console.log('❌ 视频未播放');
      }
      
      if (playingState.result.value.currentTime > 0.5) {
        console.log('✅ 视频时间正在前进');
      } else {
        console.log('❌ 视频时间未前进');
      }
    }
    
    const hasTimelineResult = typeof timelineState !== 'undefined' && timelineState && timelineState.result && timelineState.result.value;
    
    if (hasTimelineResult) {
      if (timelineState.result.value.found) {
        console.log('✅ 找到时间线组件');
        
        if (timelineState.result.value.indicatorFound) {
          console.log('✅ 找到时间指示器');
          
          const position = timelineState.result.value.indicatorPosition;
          if (position && (
              (position.left && position.left !== '0px' && position.left !== '0%') ||
              (position.transform && position.transform !== 'none' && position.transform !== 'matrix(1, 0, 0, 1, 0, 0)')
          )) {
            console.log('✅ 时间指示器已经移动');
            console.log('  - 位置信息:', position);
          } else {
            console.log('❌ 时间指示器未移动');
            console.log('  - 位置信息:', position);
          }
        } else {
          console.log('❌ 未找到时间指示器');
        }
      } else {
        console.log('❌ 未找到时间线组件');
      }
    } else {
      console.log('❌ 无法获取时间线状态');
    }
    
    console.log('\n可以检查截图文件查看测试结果:');
    console.log(`- 初始页面: ${initialScreenshotPath}`);
    console.log(`- 最终状态: ${finalScreenshotPath}`);
    
  } catch (error) {
    console.error('测试过程中发生错误:', error);
  } finally {
    if (client) {
      await client.close();
    }
  }
}

// 运行测试
console.log('开始测试事件总线架构的播放器...');
testPlayer()
  .then(() => {
    console.log('测试完成');
    process.exit(0);
  })
  .catch(error => {
    console.error('测试失败:', error);
    process.exit(1);
  }); 