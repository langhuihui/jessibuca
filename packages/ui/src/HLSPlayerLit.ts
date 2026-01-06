import { LitElement, html, css, PropertyValueMap } from 'lit';
import { HLSPlayer, HLSPlayerOptions, TimeRange } from './HLSPlayer';

// Default translations
const defaultTranslations = {
  play: '播放',
  pause: '暂停',
  loading: '加载中...',
  progressLabel: '视频进度条',
};

type Translations = typeof defaultTranslations;

/**
 * Jessibuca HLS Player Web Component
 * 
 * A Lit-based web component that provides HLS video playback functionality
 * with custom controls and timeline visualization.
 * 
 * @example
 * ```html
 * <jessibuca 
 *   src="https://example.com/video.m3u8"
 *   show-playback-rate
 *   show-progress
 *   auto-generate-ui>
 * </jessibuca>
 * ```
 * 
 * @fires play - Dispatched when playback starts
 * @fires pause - Dispatched when playback pauses
 * @fires timeupdate - Dispatched when playback time updates
 * @fires error - Dispatched when an error occurs
 */
export class JessibucaPlayer extends LitElement {
  static get properties() {
    return {
      src: { type: String },
      showPlaybackRate: { type: Boolean, attribute: 'show-playback-rate' },
      showProgress: { type: Boolean, attribute: 'show-progress' },
      autoGenerateUI: { type: Boolean, attribute: 'auto-generate-ui' },
      timeRangeMode: { type: Boolean, attribute: 'time-range-mode' },
      timeRanges: { type: Array },
      playbackRates: { type: Array },
      debug: { type: Boolean },
      showTimeRanges: { type: Boolean, attribute: 'show-time-ranges' },
      showMediaTimeline: { type: Boolean, attribute: 'show-media-timeline' },
      autoplay: { type: Boolean },
      lang: { type: String },
      // Internal state
      _isPlaying: { type: Boolean, state: true },
      _currentTime: { type: Number, state: true },
      _duration: { type: Number, state: true },
      _currentRate: { type: Number, state: true },
      _isLoading: { type: Boolean, state: true },
      _errorMessage: { type: String, state: true },
      _progressInfo: { type: String, state: true },
    };
  }

  static styles = css`
    :host {
      display: block;
      width: 100%;
      position: relative;
    }

    .player-container {
      width: 100%;
      background: #000;
      position: relative;
      aspect-ratio: 16/9;
    }

    video {
      width: 100%;
      height: 100%;
      object-fit: contain;
    }

    .controls {
      position: absolute;
      bottom: 0;
      left: 0;
      right: 0;
      padding: 10px;
      background: rgba(0, 0, 0, 0.7);
      display: flex;
      align-items: center;
      gap: 10px;
      color: white;
      font-family: system-ui, -apple-system, sans-serif;
    }

    .controls button {
      background: rgba(255, 255, 255, 0.2);
      border: none;
      color: white;
      padding: 8px 16px;
      border-radius: 4px;
      cursor: pointer;
      font-size: 14px;
      transition: background 0.2s;
    }

    .controls button:hover {
      background: rgba(255, 255, 255, 0.3);
    }

    .controls button:active {
      background: rgba(255, 255, 255, 0.4);
    }

    .controls button.active,
    .controls button.rate-active {
      background: rgba(24, 160, 88, 0.8);
    }

    .progress-container {
      flex: 1;
      display: flex;
      flex-direction: column;
      gap: 4px;
    }

    .progress-info {
      font-size: 11px;
      color: rgba(255, 255, 255, 0.7);
      white-space: nowrap;
      overflow: hidden;
      text-overflow: ellipsis;
    }

    .progress-wrapper {
      position: relative;
      height: 20px;
      flex: 1;
    }

    .progress-canvas {
      position: absolute;
      top: 0;
      left: 0;
      width: 100%;
      height: 100%;
      z-index: 1;
      border-radius: 10px;
      overflow: hidden;
    }

    .progress-input {
      position: absolute;
      top: 0;
      left: 0;
      width: 100%;
      height: 100%;
      margin: 0;
      opacity: 0;
      z-index: 2;
      cursor: pointer;
    }

    .time-display {
      font-size: 14px;
      white-space: nowrap;
      min-width: 100px;
      text-align: right;
    }

    .rate-container {
      display: flex;
      gap: 4px;
    }

    .loading-indicator {
      position: absolute;
      top: 50%;
      left: 50%;
      transform: translate(-50%, -50%);
      color: white;
      font-size: 16px;
      background: rgba(0, 0, 0, 0.7);
      padding: 10px 20px;
      border-radius: 4px;
    }

    .error-message {
      position: absolute;
      top: 50%;
      left: 50%;
      transform: translate(-50%, -50%);
      color: #ff4444;
      font-size: 14px;
      background: rgba(0, 0, 0, 0.9);
      padding: 15px 25px;
      border-radius: 4px;
      max-width: 80%;
      text-align: center;
    }
  `;

  // Public properties
  src = '';
  showPlaybackRate = true;
  showProgress = true;
  autoGenerateUI = true;
  timeRangeMode = false;
  timeRanges: TimeRange[] = [];
  playbackRates: number[] = [0.5, 0.75, 1, 1.25, 1.5, 2];
  debug = false;
  showTimeRanges = false;
  showMediaTimeline = false;
  autoplay = false;
  lang = 'zh-CN'; // Default language

  // Internal state
  _isPlaying = false;
  _currentTime = 0;
  _duration = 0;
  _currentRate = 1;
  _isLoading = false;
  _errorMessage = '';
  _progressInfo = '';

  // Private members
  private player?: HLSPlayer;
  private progressUpdateInterval?: number;
  private videoElement?: HTMLVideoElement;
  private progressInput?: HTMLInputElement;
  private progressCanvas?: HTMLCanvasElement;
  private translations: Translations = defaultTranslations;

  // Localization support
  private static locales: Record<string, Partial<Translations>> = {
    'zh-CN': {
      play: '播放',
      pause: '暂停',
      loading: '加载中...',
      progressLabel: '视频进度条',
    },
    'en-US': {
      play: 'Play',
      pause: 'Pause',
      loading: 'Loading...',
      progressLabel: 'Video progress bar',
    },
  };

  constructor() {
    super();
    this.updateTranslations();
  }

  /**
   * Update translations based on current language
   */
  private updateTranslations() {
    const localeData = JessibucaPlayer.locales[this.lang] || JessibucaPlayer.locales['zh-CN'];
    this.translations = { ...defaultTranslations, ...localeData };
  }

  /**
   * Lifecycle: Called when element is connected to DOM
   */
  connectedCallback() {
    super.connectedCallback();
    this.log('Component connected to DOM');
  }

  /**
   * Lifecycle: Called when element is disconnected from DOM
   */
  disconnectedCallback() {
    super.disconnectedCallback();
    this.cleanup();
    this.log('Component disconnected from DOM');
  }

  /**
   * Lifecycle: Called after first render
   */
  protected firstUpdated(_changedProperties: PropertyValueMap<any> | Map<PropertyKey, unknown>): void {
    this.log('First update complete, initializing player');
    
    // Get references to elements
    this.videoElement = this.shadowRoot?.querySelector('video') as HTMLVideoElement;
    this.progressInput = this.shadowRoot?.querySelector('.progress-input') as HTMLInputElement;
    this.progressCanvas = this.shadowRoot?.querySelector('.progress-canvas') as HTMLCanvasElement;
    
    this.initializePlayer();
  }

  /**
   * Lifecycle: Called when properties change
   */
  protected updated(changedProperties: PropertyValueMap<any> | Map<PropertyKey, unknown>): void {
    if (changedProperties.has('src') && this.player && this.src) {
      const oldSrc = changedProperties.get('src');
      if (oldSrc && oldSrc !== this.src) {
        this.log(`Source changed from ${oldSrc} to ${this.src}`);
        this.loadSource();
      }
    }

    if (changedProperties.has('timeRanges') && this.player) {
      this.log('Time ranges updated');
      this.player.setTimeRanges(this.timeRanges);
    }

    if (changedProperties.has('lang')) {
      this.log(`Language changed to ${this.lang}`);
      this.updateTranslations();
    }
  }

  /**
   * Initialize the HLS player
   */
  private async initializePlayer() {
    if (!this.videoElement) {
      this._errorMessage = 'Video element not found';
      this.log('Error: Video element not found', 'error');
      return;
    }

    try {
      // Create player options
      const options: HLSPlayerOptions = {
        showPlaybackRate: this.showPlaybackRate,
        showProgress: this.showProgress,
        autoGenerateUI: false, // We handle UI in the component
        timeRangeMode: this.timeRangeMode,
        timeRanges: this.timeRanges,
        playbackRates: this.playbackRates,
        debug: {
          enabled: this.debug,
          showTimeRanges: this.showTimeRanges,
          showMediaTimeline: this.showMediaTimeline,
        },
      };

      // Create player instance
      this.player = new HLSPlayer(this.videoElement, options);

      // Setup video event listeners
      this.setupVideoListeners();

      // Start progress update loop
      this.startProgressUpdate();

      // Load source if provided
      if (this.src) {
        await this.loadSource();
      }

      // Autoplay if enabled
      if (this.autoplay) {
        await this.play();
      }

      this.log('Player initialized successfully');
    } catch (error) {
      this._errorMessage = `Failed to initialize player: ${error}`;
      this.log(`Initialization error: ${error}`, 'error');
      this.dispatchEvent(new CustomEvent('error', {
        detail: { error },
        bubbles: true,
        composed: true,
      }));
    }
  }

  /**
   * Setup video element event listeners
   */
  private setupVideoListeners() {
    if (!this.videoElement) return;

    this.videoElement.addEventListener('play', () => {
      this._isPlaying = true;
      this.dispatchEvent(new CustomEvent('play', { bubbles: true, composed: true }));
    });

    this.videoElement.addEventListener('pause', () => {
      this._isPlaying = false;
      this.dispatchEvent(new CustomEvent('pause', { bubbles: true, composed: true }));
    });

    this.videoElement.addEventListener('timeupdate', () => {
      this.updateProgress();
      this.dispatchEvent(new CustomEvent('timeupdate', {
        detail: { currentTime: this._currentTime, duration: this._duration },
        bubbles: true,
        composed: true,
      }));
    });

    this.videoElement.addEventListener('loadedmetadata', () => {
      this.updateProgress();
    });

    this.videoElement.addEventListener('error', (e) => {
      const error = this.videoElement?.error;
      this._errorMessage = `Video error: ${error?.message || 'Unknown error'}`;
      this.log(`Video error: ${error?.message}`, 'error');
      this.dispatchEvent(new CustomEvent('error', {
        detail: { error },
        bubbles: true,
        composed: true,
      }));
    });

    this.videoElement.addEventListener('waiting', () => {
      this._isLoading = true;
    });

    this.videoElement.addEventListener('canplay', () => {
      this._isLoading = false;
    });
  }

  /**
   * Load video source
   */
  private async loadSource() {
    if (!this.player || !this.src) return;

    this._isLoading = true;
    this._errorMessage = '';

    try {
      this.log(`Loading source: ${this.src}`);
      await this.player.load(this.src);
      this.updateProgress();
      this.log('Source loaded successfully');
    } catch (error) {
      this._errorMessage = `Failed to load source: ${error}`;
      this.log(`Load error: ${error}`, 'error');
      this.dispatchEvent(new CustomEvent('error', {
        detail: { error },
        bubbles: true,
        composed: true,
      }));
    } finally {
      this._isLoading = false;
    }
  }

  /**
   * Start progress update loop
   */
  private startProgressUpdate() {
    if (this.progressUpdateInterval) {
      clearInterval(this.progressUpdateInterval);
    }

    this.progressUpdateInterval = window.setInterval(() => {
      this.updateProgress();
      this.updateProgressCanvas();
    }, 1000 / 30); // 30 FPS
  }

  /**
   * Update progress state
   */
  private updateProgress() {
    if (!this.player) return;

    this._currentTime = this.player.getCurrentTime();
    this._duration = this.player.getDuration();
    this._currentRate = this.player.getPlaybackRate();

    // Update progress info
    const bufferedInfo = this.getBufferedInfo();
    const progress = this._duration > 0 ? (this._currentTime / this._duration * 100).toFixed(1) : '0.0';
    this._progressInfo = `${this.formatTime(this._currentTime)} / ${this.formatTime(this._duration)} (${progress}%) ${bufferedInfo}`;
  }

  /**
   * Get buffered ranges info
   */
  private getBufferedInfo(): string {
    if (!this.videoElement || this.videoElement.buffered.length === 0) {
      return '';
    }

    const ranges: string[] = [];
    for (let i = 0; i < this.videoElement.buffered.length; i++) {
      const start = this.videoElement.buffered.start(i);
      const end = this.videoElement.buffered.end(i);
      ranges.push(`[${start.toFixed(1)}-${end.toFixed(1)}s]`);
    }
    return `Buffer: ${ranges.join(', ')}`;
  }

  /**
   * Update progress canvas visualization
   */
  private updateProgressCanvas() {
    if (!this.progressCanvas) return;

    const canvas = this.progressCanvas;
    const ctx = canvas.getContext('2d');
    if (!ctx) return;

    // Set canvas size
    canvas.width = canvas.offsetWidth * window.devicePixelRatio;
    canvas.height = canvas.offsetHeight * window.devicePixelRatio;

    // Clear canvas
    ctx.clearRect(0, 0, canvas.width, canvas.height);

    // Draw background
    ctx.fillStyle = 'rgba(255, 255, 255, 0.1)';
    ctx.fillRect(0, 0, canvas.width, canvas.height);

    if (this._duration <= 0) return;

    // Draw buffered ranges
    if (this.videoElement && this.videoElement.buffered.length > 0) {
      ctx.fillStyle = 'rgba(255, 255, 255, 0.3)';
      for (let i = 0; i < this.videoElement.buffered.length; i++) {
        const start = this.videoElement.buffered.start(i);
        const end = this.videoElement.buffered.end(i);
        const startX = (start / this._duration) * canvas.width;
        const width = ((end - start) / this._duration) * canvas.width;
        ctx.fillRect(startX, 0, width, canvas.height);
      }
    }

    // Draw played region
    if (this._currentTime > 0) {
      ctx.fillStyle = 'rgba(24, 160, 88, 0.6)';
      const playedWidth = (this._currentTime / this._duration) * canvas.width;
      ctx.fillRect(0, 0, playedWidth, canvas.height);

      // Draw current position indicator
      ctx.fillStyle = '#18a058';
      ctx.fillRect(playedWidth - 1, 0, 2, canvas.height);
    }

    // Draw time ranges if in time range mode
    if (this.timeRangeMode && this.timeRanges.length > 0) {
      const totalStart = Math.min(...this.timeRanges.map(r => r.start));
      const totalEnd = Math.max(...this.timeRanges.map(r => r.end));
      const totalDuration = totalEnd - totalStart;

      ctx.fillStyle = 'rgba(100, 160, 255, 0.4)';
      this.timeRanges.forEach(range => {
        const startX = ((range.start - totalStart) / totalDuration) * canvas.width;
        const width = ((range.end - range.start) / totalDuration) * canvas.width;
        ctx.fillRect(startX, 0, width, canvas.height);
      });
    }
  }

  /**
   * Format time in seconds to HH:MM:SS or MM:SS
   */
  private formatTime(seconds: number): string {
    if (isNaN(seconds) || !isFinite(seconds) || seconds < 0) {
      return '00:00';
    }

    const hours = Math.floor(seconds / 3600);
    const minutes = Math.floor((seconds % 3600) / 60);
    const secs = Math.floor(seconds % 60);

    if (hours > 0) {
      return `${hours.toString().padStart(2, '0')}:${minutes.toString().padStart(2, '0')}:${secs.toString().padStart(2, '0')}`;
    }
    return `${minutes.toString().padStart(2, '0')}:${secs.toString().padStart(2, '0')}`;
  }

  /**
   * Toggle play/pause
   */
  private async togglePlay() {
    if (!this.player) return;

    try {
      if (this._isPlaying) {
        this.player.pause();
      } else {
        await this.player.play();
      }
    } catch (error) {
      this.log(`Play/pause error: ${error}`, 'error');
    }
  }

  /**
   * Handle seek from progress bar
   */
  private handleSeek(e: Event) {
    if (!this.player || !this.progressInput) return;

    const value = Number(this.progressInput.value);
    const seekTime = (value / 100) * this._duration;

    this.log(`Seeking to ${seekTime.toFixed(2)}s (${value}%)`);
    this.player.seek(seekTime);
  }

  /**
   * Set playback rate
   */
  private setPlaybackRate(rate: number) {
    if (!this.player) return;

    this.player.setPlaybackRate(rate);
    this._currentRate = rate;
    this.log(`Playback rate set to ${rate}x`);
  }

  /**
   * Cleanup resources
   */
  private cleanup() {
    if (this.progressUpdateInterval) {
      clearInterval(this.progressUpdateInterval);
      this.progressUpdateInterval = undefined;
    }

    if (this.player) {
      this.player.destroy();
      this.player = undefined;
    }
  }

  /**
   * Log message
   */
  private log(message: string, level: 'info' | 'error' = 'info') {
    if (this.debug || level === 'error') {
      const prefix = '[JessibucaPlayer]';
      if (level === 'error') {
        console.error(`${prefix} ${message}`);
      } else {
        console.log(`${prefix} ${message}`);
      }
    }
  }

  /**
   * Public API: Play video
   */
  public async play() {
    if (this.player) {
      await this.player.play();
    }
  }

  /**
   * Public API: Pause video
   */
  public pause() {
    if (this.player) {
      this.player.pause();
    }
  }

  /**
   * Public API: Seek to time
   */
  public seek(time: number) {
    if (this.player) {
      this.player.seek(time);
    }
  }

  /**
   * Public API: Get current time
   */
  public getCurrentTime(): number {
    return this.player?.getCurrentTime() || 0;
  }

  /**
   * Public API: Get duration
   */
  public getDuration(): number {
    return this.player?.getDuration() || 0;
  }

  /**
   * Render the component
   */
  render() {
    const progressValue = this._duration > 0 ? (this._currentTime / this._duration * 100) : 0;

    return html`
      <div class="player-container">
        <video></video>

        ${this.autoGenerateUI ? html`
          <div class="controls">
            <button @click=${this.togglePlay}>
              ${this._isPlaying ? this.translations.pause : this.translations.play}
            </button>

            ${this.showProgress ? html`
              <div class="progress-container">
                ${this.debug ? html`
                  <div class="progress-info">${this._progressInfo}</div>
                ` : ''}
                <div class="progress-wrapper">
                  <canvas class="progress-canvas"></canvas>
                  <input
                    type="range"
                    class="progress-input"
                    min="0"
                    max="100"
                    .value=${progressValue.toString()}
                    @input=${this.handleSeek}
                    aria-label=${this.translations.progressLabel}
                  />
                </div>
              </div>
              <div class="time-display">
                ${this.formatTime(this._currentTime)} / ${this.formatTime(this._duration)}
              </div>
            ` : ''}

            ${this.showPlaybackRate ? html`
              <div class="rate-container">
                ${this.playbackRates.map(rate => html`
                  <button
                    class=${rate === this._currentRate ? 'rate-active' : ''}
                    @click=${() => this.setPlaybackRate(rate)}
                  >
                    ${rate}x
                  </button>
                `)}
              </div>
            ` : ''}
          </div>
        ` : ''}

        ${this._isLoading ? html`
          <div class="loading-indicator">${this.translations.loading}</div>
        ` : ''}

        ${this._errorMessage ? html`
          <div class="error-message">${this._errorMessage}</div>
        ` : ''}
      </div>
    `;
  }

  /**
   * Register a new locale for the player
   * @param locale - Locale identifier (e.g., 'fr-FR', 'de-DE')
   * @param translations - Translation object
   */
  public static registerLocale(locale: string, translations: Partial<Translations>) {
    JessibucaPlayer.locales[locale] = translations;
  }
}

// Register the custom element
customElements.define('jessibuca', JessibucaPlayer);

// Export types for users who want to add custom translations
export type { Translations };

declare global {
  interface HTMLElementTagNameMap {
    'jessibuca': JessibucaPlayer;
  }
}
