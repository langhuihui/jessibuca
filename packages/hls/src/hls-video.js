/**
 * HLS Video Web Component
 * 一个封装了 HLS.js 的自定义元素，提供类似 Safari 原生 HLS 播放的简单用法
 * 
 * 使用方法:
 * <hls-video src="path/to/playlist.m3u8" controls autoplay muted></hls-video>
 */

class HlsVideo extends HTMLElement {
  constructor() {
    super();

    // 创建 Shadow DOM
    this.attachShadow({ mode: 'open' });

    // 初始化状态
    this.hls = null;
    this.video = null;
    this.isHlsSupported = false;
    this.isNativeHlsSupported = false;

    // 绑定方法
    this.handleError = this.handleError.bind(this);
    this.handleLoadStart = this.handleLoadStart.bind(this);
    this.handleCanPlay = this.handleCanPlay.bind(this);

    // 创建样式和模板
    this.createTemplate();

    // 检查 HLS 支持
    this.checkHlsSupport();
  }

  static get observedAttributes() {
    return [
      'src', 'controls', 'autoplay', 'muted', 'loop', 'poster',
      'width', 'height', 'preload', 'crossorigin', 'playsinline',
      'volume', 'playback-rate', 'current-time', 'quality-level',
      'debug', 'low-latency', 'max-buffer-length'
    ];
  }

  createTemplate() {
    const template = document.createElement('template');
    template.innerHTML = `
      <style>
        :host {
          display: inline-block;
          position: relative;
          max-width: 100%;
        }
        
        video {
          width: 100%;
          height: 100%;
          display: block;
        }
        
        .error-message {
          position: absolute;
          top: 50%;
          left: 50%;
          transform: translate(-50%, -50%);
          background: rgba(220, 53, 69, 0.9);
          color: white;
          padding: 12px 20px;
          border-radius: 4px;
          font-family: Arial, sans-serif;
          font-size: 14px;
          text-align: center;
          max-width: 80%;
          z-index: 10;
        }
        
        .loading {
          position: absolute;
          top: 50%;
          left: 50%;
          transform: translate(-50%, -50%);
          background: rgba(0, 0, 0, 0.7);
          color: white;
          padding: 12px 20px;
          border-radius: 4px;
          font-family: Arial, sans-serif;
          font-size: 14px;
          z-index: 5;
        }
        
        .quality-selector {
          position: absolute;
          bottom: 10px;
          right: 10px;
          background: rgba(0, 0, 0, 0.7);
          border-radius: 4px;
          padding: 5px;
          z-index: 15;
        }
        
        .quality-selector select {
          background: transparent;
          color: white;
          border: none;
          padding: 2px 5px;
          font-size: 12px;
        }
        
        .quality-selector select option {
          background: black;
          color: white;
        }
        
        :host([hidden]) {
          display: none !important;
        }
      </style>
      
      <video part="video"></video>
      <div class="loading" part="loading" style="display: none;">加载中...</div>
      <div class="error-message" part="error" style="display: none;"></div>
      <div class="quality-selector" part="quality-selector" style="display: none;">
        <select part="quality-select">
          <option value="-1">自动</option>
        </select>
      </div>
    `;

    this.shadowRoot.appendChild(template.content.cloneNode(true));

    // 获取模板元素引用
    this.video = this.shadowRoot.querySelector('video');
    this.loadingElement = this.shadowRoot.querySelector('.loading');
    this.errorElement = this.shadowRoot.querySelector('.error-message');
    this.qualitySelector = this.shadowRoot.querySelector('.quality-selector');
    this.qualitySelect = this.shadowRoot.querySelector('select');

    // 绑定事件
    this.setupVideoEvents();
    this.setupQualitySelector();
  }

  async checkHlsSupport() {
    // 检查 HLS.js 支持
    if (typeof window.Hls !== 'undefined') {
      this.isHlsSupported = window.Hls.isSupported();
    } else {
      // 动态加载 HLS.js
      try {
        await this.loadHlsJs();
        this.isHlsSupported = window.Hls.isSupported();
      } catch (error) {
        console.warn('Failed to load HLS.js:', error);
      }
    }

    // 检查原生 HLS 支持
    this.isNativeHlsSupported = this.video.canPlayType('application/vnd.apple.mpegurl') !== '';
  }

  async loadHlsJs() {
    return new Promise((resolve, reject) => {
      if (typeof window.Hls !== 'undefined') {
        resolve();
        return;
      }

      const script = document.createElement('script');
      script.src = 'https://cdn.jsdelivr.net/npm/hls.js@latest';
      script.onload = resolve;
      script.onerror = reject;
      document.head.appendChild(script);
    });
  }

  setupVideoEvents() {
    this.video.addEventListener('loadstart', this.handleLoadStart);
    this.video.addEventListener('canplay', this.handleCanPlay);
    this.video.addEventListener('error', this.handleError);

    // 代理常用的视频事件
    const eventsToProxy = [
      'loadstart', 'loadeddata', 'loadedmetadata', 'canplay', 'canplaythrough',
      'play', 'pause', 'playing', 'waiting', 'seeking', 'seeked', 'ended',
      'timeupdate', 'progress', 'durationchange', 'volumechange', 'ratechange',
      'resize', 'enterpictureinpicture', 'leavepictureinpicture'
    ];

    eventsToProxy.forEach(eventName => {
      this.video.addEventListener(eventName, (event) => {
        this.dispatchEvent(new CustomEvent(eventName, {
          detail: event,
          bubbles: false
        }));
      });
    });
  }

  setupQualitySelector() {
    this.qualitySelect.addEventListener('change', () => {
      const level = parseInt(this.qualitySelect.value);
      this.setQualityLevel(level);
    });
  }

  connectedCallback() {
    // 元素被添加到 DOM 时
    this.updateVideoAttributes();
    if (this.getAttribute('src')) {
      this.loadSource();
    }
  }

  disconnectedCallback() {
    // 元素被移除时清理
    this.destroyHls();
  }

  attributeChangedCallback(name, oldValue, newValue) {
    if (oldValue === newValue) return;

    switch (name) {
      case 'src':
        if (newValue) {
          this.loadSource();
        }
        break;
      case 'quality-level':
        this.setQualityLevel(parseInt(newValue) || -1);
        break;
      default:
        this.updateVideoAttributes();
        break;
    }
  }

  updateVideoAttributes() {
    // 同步属性到内部 video 元素
    const videoAttributes = [
      'controls', 'autoplay', 'muted', 'loop', 'poster',
      'width', 'height', 'preload', 'crossorigin', 'playsinline'
    ];

    videoAttributes.forEach(attr => {
      if (this.hasAttribute(attr)) {
        const value = this.getAttribute(attr);
        if (value === '' || value === attr) {
          this.video.setAttribute(attr, '');
        } else {
          this.video.setAttribute(attr, value);
        }
      } else {
        this.video.removeAttribute(attr);
      }
    });

    // 处理数值属性
    const volume = this.getAttribute('volume');
    if (volume !== null) {
      this.video.volume = parseFloat(volume);
    }

    const playbackRate = this.getAttribute('playback-rate');
    if (playbackRate !== null) {
      this.video.playbackRate = parseFloat(playbackRate);
    }

    const currentTime = this.getAttribute('current-time');
    if (currentTime !== null) {
      this.video.currentTime = parseFloat(currentTime);
    }
  }

  async loadSource() {
    const src = this.getAttribute('src');
    if (!src) return;

    this.showLoading();
    this.hideError();

    try {
      await this.checkHlsSupport();

      if (this.isHlsUrl(src)) {
        if (this.isHlsSupported) {
          await this.loadWithHlsJs(src);
        } else if (this.isNativeHlsSupported) {
          this.loadWithNativeHls(src);
        } else {
          throw new Error('您的浏览器不支持 HLS 播放');
        }
      } else {
        // 非 HLS 源，直接使用 video 元素
        this.video.src = src;
      }
    } catch (error) {
      this.showError(error.message);
    }
  }

  isHlsUrl(url) {
    return url.includes('.m3u8') || url.includes('application/vnd.apple.mpegurl');
  }

  async loadWithHlsJs(src) {
    this.destroyHls();

    const config = this.getHlsConfig();
    this.hls = new window.Hls(config);

    this.setupHlsEvents();
    this.hls.loadSource(src);
    this.hls.attachMedia(this.video);
  }

  loadWithNativeHls(src) {
    this.video.src = src;
    this.hideLoading();
  }

  getHlsConfig() {
    const debug = this.hasAttribute('debug');
    const lowLatency = this.hasAttribute('low-latency');
    const maxBufferLength = parseInt(this.getAttribute('max-buffer-length')) || 30;

    return {
      debug,
      lowLatencyMode: lowLatency,
      maxBufferLength,
      enableWorker: true,
      autoStartLoad: true,
      startPosition: -1,
      capLevelToPlayerSize: false
    };
  }

  setupHlsEvents() {
    if (!this.hls) return;

    this.hls.on(window.Hls.Events.MEDIA_ATTACHED, () => {
      console.log('HLS: Media attached');
    });

    this.hls.on(window.Hls.Events.MANIFEST_PARSED, (event, data) => {
      console.log('HLS: Manifest parsed', data.levels.length, 'quality levels');
      this.hideLoading();
      this.updateQualitySelector(data.levels);

      // 触发自定义事件
      this.dispatchEvent(new CustomEvent('hlsmanifestparsed', {
        detail: { levels: data.levels }
      }));
    });

    this.hls.on(window.Hls.Events.LEVEL_SWITCHED, (event, data) => {
      console.log('HLS: Level switched to', data.level);
      this.qualitySelect.value = data.level.toString();

      this.dispatchEvent(new CustomEvent('hlslevelswitched', {
        detail: { level: data.level }
      }));
    });

    this.hls.on(window.Hls.Events.ERROR, (event, data) => {
      console.error('HLS Error:', data);

      if (data.fatal) {
        switch (data.type) {
          case window.Hls.ErrorTypes.NETWORK_ERROR:
            console.log('尝试恢复网络错误...');
            this.hls.startLoad();
            break;
          case window.Hls.ErrorTypes.MEDIA_ERROR:
            console.log('尝试恢复媒体错误...');
            this.hls.recoverMediaError();
            break;
          default:
            this.showError(`播放错误: ${data.details}`);
            this.destroyHls();
            break;
        }
      }

      this.dispatchEvent(new CustomEvent('hlserror', {
        detail: data
      }));
    });
  }

  updateQualitySelector(levels) {
    // 清空现有选项
    this.qualitySelect.innerHTML = '<option value="-1">自动</option>';

    // 添加质量级别选项
    levels.forEach((level, index) => {
      const option = document.createElement('option');
      option.value = index.toString();
      option.textContent = `${level.height}p (${Math.round(level.bitrate / 1000)}k)`;
      this.qualitySelect.appendChild(option);
    });

    // 显示质量选择器（如果有多个级别）
    if (levels.length > 1) {
      this.qualitySelector.style.display = 'block';
    }
  }

  setQualityLevel(level) {
    if (this.hls) {
      this.hls.currentLevel = level;
    }
  }

  destroyHls() {
    if (this.hls) {
      this.hls.destroy();
      this.hls = null;
    }
  }

  showLoading() {
    this.loadingElement.style.display = 'block';
  }

  hideLoading() {
    this.loadingElement.style.display = 'none';
  }

  showError(message) {
    this.errorElement.textContent = message;
    this.errorElement.style.display = 'block';
    this.hideLoading();
  }

  hideError() {
    this.errorElement.style.display = 'none';
  }

  handleLoadStart() {
    this.hideError();
  }

  handleCanPlay() {
    this.hideLoading();
  }

  handleError(event) {
    const error = this.video.error;
    if (error) {
      let message = '视频播放出错';
      switch (error.code) {
        case error.MEDIA_ERR_ABORTED:
          message = '视频播放被中止';
          break;
        case error.MEDIA_ERR_NETWORK:
          message = '网络错误导致视频下载失败';
          break;
        case error.MEDIA_ERR_DECODE:
          message = '视频解码失败';
          break;
        case error.MEDIA_ERR_SRC_NOT_SUPPORTED:
          message = '视频格式不支持';
          break;
      }
      this.showError(message);
    }
  }

  // 公开的 API 方法
  play() {
    return this.video.play();
  }

  pause() {
    this.video.pause();
  }

  load() {
    this.video.load();
  }

  // 获取当前质量级别
  get currentLevel() {
    return this.hls ? this.hls.currentLevel : -1;
  }

  // 设置质量级别
  set currentLevel(level) {
    this.setQualityLevel(level);
  }

  // 获取可用的质量级别
  get levels() {
    return this.hls ? this.hls.levels : [];
  }

  // 获取/设置音量
  get volume() {
    return this.video.volume;
  }

  set volume(value) {
    this.video.volume = value;
  }

  // 获取/设置当前时间
  get currentTime() {
    return this.video.currentTime;
  }

  set currentTime(value) {
    this.video.currentTime = value;
  }

  // 获取持续时间
  get duration() {
    return this.video.duration;
  }

  // 获取/设置播放速率
  get playbackRate() {
    return this.video.playbackRate;
  }

  set playbackRate(value) {
    this.video.playbackRate = value;
  }

  // 获取是否暂停
  get paused() {
    return this.video.paused;
  }

  // 获取是否结束
  get ended() {
    return this.video.ended;
  }

  // 获取缓冲范围
  get buffered() {
    return this.video.buffered;
  }

  // 获取网络状态
  get networkState() {
    return this.video.networkState;
  }

  // 获取就绪状态
  get readyState() {
    return this.video.readyState;
  }
}

// 注册自定义元素
customElements.define('hls-video', HlsVideo);

// 如果在模块环境中，导出类
if (typeof module !== 'undefined' && module.exports) {
  module.exports = HlsVideo;
}
