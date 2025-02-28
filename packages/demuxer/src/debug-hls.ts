import { HLSv7Demuxer } from './hlsv7';

// 配置测试URL和视频元素
const testHlsUrl = 'http://localhost:8080/hls/vod/fmp4.m3u8?start=1740972167&streamPath=live/video';
const videoElement = document.createElement('video');
videoElement.controls = true;
videoElement.width = 640;
videoElement.height = 360;
document.body.appendChild(videoElement);

// 性能监控数据
interface PerformanceMetrics {
  initTime: number;
  seekTimes: number[];
  bufferSizes: number[];
  errors: string[];
  segmentLoadTimes: Record<number, number>;
  memoryUsage: number[];
  playbackStats: {
    freezeCount: number;
    rebufferingTime: number;
    playbackRate: number[];
  };
}

const metrics: PerformanceMetrics = {
  initTime: 0,
  seekTimes: [],
  bufferSizes: [],
  errors: [],
  segmentLoadTimes: {},
  memoryUsage: [],
  playbackStats: {
    freezeCount: 0,
    rebufferingTime: 0,
    playbackRate: []
  }
};

// 创建UI
const container = document.createElement('div');
container.style.fontFamily = 'Arial, sans-serif';
container.style.padding = '20px';
container.style.maxWidth = '800px';
container.style.margin = '0 auto';
document.body.appendChild(container);

// 标题
const title = document.createElement('h2');
title.textContent = 'HLSv7Demuxer 性能测试';
container.appendChild(title);

// 状态指示器
const statusIndicator = document.createElement('div');
statusIndicator.style.marginBottom = '20px';
statusIndicator.style.padding = '10px';
statusIndicator.style.borderRadius = '4px';
statusIndicator.style.backgroundColor = '#f0f0f0';
container.appendChild(statusIndicator);

// 控制按钮容器
const controls = document.createElement('div');
controls.style.display = 'flex';
controls.style.gap = '10px';
controls.style.marginBottom = '20px';
container.appendChild(controls);

// 初始化按钮
const initButton = createButton('初始化播放器');
controls.appendChild(initButton);

// 播放按钮
const playButton = createButton('播放');
controls.appendChild(playButton);

// 暂停按钮
const pauseButton = createButton('暂停');
controls.appendChild(pauseButton);

// 跳转按钮 - 10秒
const seek10Button = createButton('跳转+10秒');
controls.appendChild(seek10Button);

// 跳转按钮 - 30秒
const seek30Button = createButton('跳转+30秒');
controls.appendChild(seek30Button);

// 跳转按钮 - 自定义
const seekCustomButton = createButton('自定义跳转');
controls.appendChild(seekCustomButton);

// 销毁按钮
const destroyButton = createButton('销毁播放器');
controls.appendChild(destroyButton);

// 性能报告按钮
const reportButton = createButton('生成性能报告');
reportButton.style.backgroundColor = '#4CAF50';
controls.appendChild(reportButton);

// 日志容器
const logContainer = document.createElement('div');
logContainer.style.border = '1px solid #ddd';
logContainer.style.padding = '10px';
logContainer.style.height = '300px';
logContainer.style.overflowY = 'scroll';
logContainer.style.backgroundColor = '#f9f9f9';
logContainer.style.fontFamily = 'monospace';
logContainer.style.fontSize = '12px';
container.appendChild(logContainer);

// 性能指标容器
const metricsContainer = document.createElement('div');
metricsContainer.style.marginTop = '20px';
metricsContainer.style.border = '1px solid #ddd';
metricsContainer.style.padding = '10px';
container.appendChild(metricsContainer);

// 更新性能指标显示
function updateMetricsDisplay() {
  metricsContainer.innerHTML = `
    <h3>性能指标</h3>
    <p>初始化时间: ${metrics.initTime.toFixed(2)} ms</p>
    <p>平均跳转时间: ${calculateAverage(metrics.seekTimes).toFixed(2)} ms</p>
    <p>平均缓冲区大小: ${calculateAverage(metrics.bufferSizes).toFixed(2)} MB</p>
    <p>错误数量: ${metrics.errors.length}</p>
    <p>卡顿次数: ${metrics.playbackStats.freezeCount}</p>
    <p>重新缓冲时间: ${metrics.playbackStats.rebufferingTime.toFixed(2)} ms</p>
    <p>平均播放速率: ${calculateAverage(metrics.playbackStats.playbackRate).toFixed(2)}x</p>
  `;
}

// 计算平均值辅助函数
function calculateAverage(arr: number[]): number {
  if (arr.length === 0) return 0;
  return arr.reduce((a, b) => a + b, 0) / arr.length;
}

// 添加日志
function addLog(message: string, type: 'info' | 'error' | 'warning' | 'success' = 'info') {
  const logEntry = document.createElement('div');
  const timestamp = new Date().toISOString().split('T')[1].replace('Z', '');

  logEntry.innerHTML = `<span style="color: #888;">[${timestamp}]</span> <span style="color: ${type === 'info' ? '#333' :
    type === 'error' ? '#d9534f' :
      type === 'warning' ? '#f0ad4e' :
        '#5cb85c'
    };">${message}</span>`;

  logContainer.appendChild(logEntry);
  logContainer.scrollTop = logContainer.scrollHeight;

  // 如果是错误，记录到指标中
  if (type === 'error') {
    metrics.errors.push(message);
  }
}

// 辅助函数 - 创建按钮
function createButton(text: string): HTMLButtonElement {
  const button = document.createElement('button');
  button.textContent = text;
  button.style.padding = '8px 16px';
  button.style.backgroundColor = '#007BFF';
  button.style.color = 'white';
  button.style.border = 'none';
  button.style.borderRadius = '4px';
  button.style.cursor = 'pointer';
  return button;
}

// 辅助函数 - 更新状态
function updateStatus(message: string, color: string = '#f0f0f0') {
  statusIndicator.textContent = message;
  statusIndicator.style.backgroundColor = color;
}

// 实例化和事件处理
let demuxer: HLSv7Demuxer | null = null;

// 初始化播放器
initButton.addEventListener('click', async () => {
  if (demuxer) {
    addLog('播放器已经初始化，请先销毁现有实例', 'warning');
    return;
  }

  try {
    updateStatus('初始化中...', '#fff8e1');
    addLog('初始化播放器...');

    const startTime = performance.now();

    // 使用重构后的构造函数，传递配置选项
    demuxer = new HLSv7Demuxer(videoElement, {
      logLevel: 'debug',
      maxBufferLength: 60,
      codec: 'video/mp4; codecs="avc1.64001f"'
    });

    // 设置事件监听器
    demuxer.on('debug', (message) => {
      addLog(message);
    });

    demuxer.on('error', (error) => {
      addLog(`错误: ${error.message}`, 'error');
    });

    demuxer.on('playlistUpdate', (playlist) => {
      addLog(`播放列表更新: ${playlist.length}个片段`, 'info');
    });

    demuxer.on('segmentLoaded', (segmentIndex) => {
      const loadTime = performance.now();
      if (!metrics.segmentLoadTimes[segmentIndex]) {
        metrics.segmentLoadTimes[segmentIndex] = loadTime;
      }
      addLog(`片段${segmentIndex + 1}加载完成`, 'success');
    });

    demuxer.on('bufferUpdate', (ranges) => {
      const totalBuffered = ranges.reduce((total, range) => total + (range.end - range.start), 0);
      metrics.bufferSizes.push(totalBuffered);

      let rangesText = ranges.map(range =>
        `${range.start.toFixed(2)}s-${range.end.toFixed(2)}s`).join(', ');

      addLog(`缓冲区更新: ${rangesText}`, 'info');
    });

    // 初始化
    await demuxer.init(testHlsUrl);

    const endTime = performance.now();
    metrics.initTime = endTime - startTime;

    updateStatus('初始化完成', '#e8f5e9');
    addLog(`初始化完成，耗时: ${metrics.initTime.toFixed(2)}ms`, 'success');
    updateMetricsDisplay();
  } catch (error) {
    updateStatus('初始化失败', '#ffebee');
    addLog(`初始化失败: ${error}`, 'error');
  }
});

// 播放
playButton.addEventListener('click', async () => {
  if (!demuxer) {
    addLog('请先初始化播放器', 'warning');
    return;
  }

  try {
    updateStatus('播放中...', '#e8f5e9');
    await demuxer.play();

    // 记录播放速率
    metrics.playbackStats.playbackRate.push(videoElement.playbackRate);

    addLog('开始播放', 'success');

    // 监控卡顿
    monitorPlaybackFreeze();
  } catch (error) {
    updateStatus('播放失败', '#ffebee');
    addLog(`播放失败: ${error}`, 'error');
  }
});

// 暂停
pauseButton.addEventListener('click', () => {
  if (!demuxer) {
    addLog('请先初始化播放器', 'warning');
    return;
  }

  try {
    demuxer.pause();
    updateStatus('已暂停', '#e3f2fd');
    addLog('已暂停播放');
  } catch (error) {
    addLog(`暂停失败: ${error}`, 'error');
  }
});

// 跳转+10秒
seek10Button.addEventListener('click', async () => {
  if (!demuxer) {
    addLog('请先初始化播放器', 'warning');
    return;
  }

  try {
    const currentTime = demuxer.getCurrentTime();
    const targetTime = currentTime + 10;

    updateStatus(`跳转中 (${currentTime.toFixed(2)}s → ${targetTime.toFixed(2)}s)...`, '#fff8e1');
    addLog(`尝试跳转: ${currentTime.toFixed(2)}s → ${targetTime.toFixed(2)}s`);

    const startTime = performance.now();

    await demuxer.seek(targetTime);

    const endTime = performance.now();
    const seekTime = endTime - startTime;
    metrics.seekTimes.push(seekTime);

    updateStatus(`跳转完成，当前位置: ${demuxer.getCurrentTime().toFixed(2)}s`, '#e8f5e9');
    addLog(`跳转完成，耗时: ${seekTime.toFixed(2)}ms`, 'success');
    updateMetricsDisplay();
  } catch (error) {
    updateStatus('跳转失败', '#ffebee');
    addLog(`跳转失败: ${error}`, 'error');
  }
});

// 跳转+30秒
seek30Button.addEventListener('click', async () => {
  if (!demuxer) {
    addLog('请先初始化播放器', 'warning');
    return;
  }

  try {
    const currentTime = demuxer.getCurrentTime();
    const targetTime = currentTime + 30;

    updateStatus(`跳转中 (${currentTime.toFixed(2)}s → ${targetTime.toFixed(2)}s)...`, '#fff8e1');
    addLog(`尝试跳转: ${currentTime.toFixed(2)}s → ${targetTime.toFixed(2)}s`);

    const startTime = performance.now();

    await demuxer.seek(targetTime);

    const endTime = performance.now();
    const seekTime = endTime - startTime;
    metrics.seekTimes.push(seekTime);

    updateStatus(`跳转完成，当前位置: ${demuxer.getCurrentTime().toFixed(2)}s`, '#e8f5e9');
    addLog(`跳转完成，耗时: ${seekTime.toFixed(2)}ms`, 'success');
    updateMetricsDisplay();
  } catch (error) {
    updateStatus('跳转失败', '#ffebee');
    addLog(`跳转失败: ${error}`, 'error');
  }
});

// 自定义跳转
seekCustomButton.addEventListener('click', async () => {
  if (!demuxer) {
    addLog('请先初始化播放器', 'warning');
    return;
  }

  const input = prompt('请输入目标时间（秒）:', '60');
  if (input === null) return;

  const targetTime = parseFloat(input);
  if (isNaN(targetTime) || targetTime < 0) {
    addLog('无效的时间值', 'warning');
    return;
  }

  try {
    const currentTime = demuxer.getCurrentTime();

    updateStatus(`跳转中 (${currentTime.toFixed(2)}s → ${targetTime.toFixed(2)}s)...`, '#fff8e1');
    addLog(`尝试跳转: ${currentTime.toFixed(2)}s → ${targetTime.toFixed(2)}s`);

    const startTime = performance.now();

    await demuxer.seek(targetTime);

    const endTime = performance.now();
    const seekTime = endTime - startTime;
    metrics.seekTimes.push(seekTime);

    updateStatus(`跳转完成，当前位置: ${demuxer.getCurrentTime().toFixed(2)}s`, '#e8f5e9');
    addLog(`跳转完成，耗时: ${seekTime.toFixed(2)}ms`, 'success');
    updateMetricsDisplay();
  } catch (error) {
    updateStatus('跳转失败', '#ffebee');
    addLog(`跳转失败: ${error}`, 'error');
  }
});

// 销毁播放器
destroyButton.addEventListener('click', () => {
  if (!demuxer) {
    addLog('播放器尚未初始化', 'warning');
    return;
  }

  try {
    demuxer.destroy();
    demuxer = null;

    updateStatus('播放器已销毁', '#f5f5f5');
    addLog('播放器已销毁', 'info');
  } catch (error) {
    addLog(`销毁失败: ${error}`, 'error');
  }
});

// 生成性能报告
reportButton.addEventListener('click', () => {
  const report = document.createElement('div');
  report.style.backgroundColor = '#fff';
  report.style.padding = '20px';
  report.style.position = 'fixed';
  report.style.top = '50%';
  report.style.left = '50%';
  report.style.transform = 'translate(-50%, -50%)';
  report.style.width = '80%';
  report.style.maxWidth = '600px';
  report.style.maxHeight = '80%';
  report.style.overflow = 'auto';
  report.style.boxShadow = '0 4px 8px rgba(0,0,0,0.1)';
  report.style.zIndex = '1000';
  report.style.borderRadius = '8px';

  report.innerHTML = `
    <h2 style="margin-top:0">HLSv7Demuxer 性能报告</h2>
    <button id="closeReport" style="position:absolute;top:10px;right:10px;background:none;border:none;font-size:20px;cursor:pointer;">×</button>
    
    <h3>基本指标</h3>
    <p>初始化时间: <strong>${metrics.initTime.toFixed(2)} ms</strong></p>
    <p>测试时长: <strong>${((performance.now() - metrics.initTime) / 1000).toFixed(2)} 秒</strong></p>
    
    <h3>跳转性能</h3>
    <p>跳转次数: <strong>${metrics.seekTimes.length}</strong></p>
    <p>平均跳转时间: <strong>${calculateAverage(metrics.seekTimes).toFixed(2)} ms</strong></p>
    <p>最长跳转时间: <strong>${Math.max(...(metrics.seekTimes.length ? metrics.seekTimes : [0])).toFixed(2)} ms</strong></p>
    <p>最短跳转时间: <strong>${Math.min(...(metrics.seekTimes.length ? metrics.seekTimes : [0])).toFixed(2)} ms</strong></p>
    
    <h3>缓冲性能</h3>
    <p>平均缓冲区大小: <strong>${calculateAverage(metrics.bufferSizes).toFixed(2)} 秒</strong></p>
    <p>最大缓冲区大小: <strong>${Math.max(...(metrics.bufferSizes.length ? metrics.bufferSizes : [0])).toFixed(2)} 秒</strong></p>
    
    <h3>片段加载</h3>
    <p>加载片段数: <strong>${Object.keys(metrics.segmentLoadTimes).length}</strong></p>
    
    <h3>错误统计</h3>
    <p>总错误数: <strong>${metrics.errors.length}</strong></p>
    ${metrics.errors.length > 0 ? `<ul>${metrics.errors.map(err => `<li>${err}</li>`).join('')}</ul>` : ''}
    
    <h3>播放质量</h3>
    <p>卡顿次数: <strong>${metrics.playbackStats.freezeCount}</strong></p>
    <p>重新缓冲时间: <strong>${metrics.playbackStats.rebufferingTime.toFixed(2)} ms</strong></p>
    
    <h3>建议</h3>
    <ul>
      ${metrics.seekTimes.some(time => time > 1000) ? '<li>跳转时间较长，考虑优化缓冲区管理</li>' : ''}
      ${metrics.errors.length > 0 ? '<li>存在错误，请检查错误日志</li>' : ''}
      ${metrics.playbackStats.freezeCount > 3 ? '<li>播放卡顿频繁，考虑增加预加载缓冲区大小</li>' : ''}
    </ul>
  `;

  document.body.appendChild(report);

  document.getElementById('closeReport')?.addEventListener('click', () => {
    document.body.removeChild(report);
  });
});

// 监控播放卡顿
let lastTime = 0;
let lastPlaybackCheck = 0;
let checkInterval: any = null;

function monitorPlaybackFreeze() {
  if (checkInterval) {
    clearInterval(checkInterval);
  }

  lastTime = videoElement.currentTime;
  lastPlaybackCheck = performance.now();

  checkInterval = setInterval(() => {
    if (!demuxer || videoElement.paused) return;

    const now = performance.now();
    const currentTime = videoElement.currentTime;

    // 检测是否卡顿 (时间过去了但视频未前进)
    if (now - lastPlaybackCheck > 500 && Math.abs(currentTime - lastTime) < 0.01) {
      metrics.playbackStats.freezeCount++;
      const freezeStartTime = now;

      addLog(`检测到卡顿！当前时间: ${currentTime.toFixed(2)}s`, 'warning');

      // 检测何时恢复正常播放
      const freezeCheckInterval = setInterval(() => {
        if (Math.abs(videoElement.currentTime - currentTime) > 0.01 || videoElement.paused) {
          clearInterval(freezeCheckInterval);
          const freezeTime = performance.now() - freezeStartTime;
          metrics.playbackStats.rebufferingTime += freezeTime;

          addLog(`播放已恢复，卡顿时长: ${freezeTime.toFixed(2)}ms`, 'success');
          updateMetricsDisplay();
        }
      }, 100);
    }

    lastTime = currentTime;
    lastPlaybackCheck = now;
  }, 1000);
}

// 清理函数
window.addEventListener('beforeunload', () => {
  if (demuxer) {
    demuxer.destroy();
  }
  if (checkInterval) {
    clearInterval(checkInterval);
  }
});

// 初始状态
updateStatus('准备就绪', '#f5f5f5');
addLog('调试页面已加载');
updateMetricsDisplay(); 