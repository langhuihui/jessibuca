import EventEmitter from "eventemitter3";
export class HLSv7Demuxer extends EventEmitter {
  private videoElement: HTMLVideoElement;
  private mediaSource: MediaSource | null = null;
  private sourceBuffer: SourceBuffer | null = null;
  private currentPlaylist: string[] = [];
  private currentIndex: number = 0;
  private pendingBuffers: ArrayBuffer[] = [];
  private isBuffering: boolean = false;
  private readonly maxBufferLength: number = 30; // 保持30秒的缓冲区
  private isDestroyed: boolean = false;
  constructor(videoElement: HTMLVideoElement) {
    super();
    this.videoElement = videoElement;
    this.setupVideoListeners();
  }

  private setupVideoListeners() {
    this.videoElement.addEventListener('error', this.handleVideoError);
    this.videoElement.addEventListener('seeking', this.handleSeeking);
    this.videoElement.addEventListener('waiting', () => this.log('视频缓冲中'));
    this.videoElement.addEventListener('canplay', () => this.log('视频可以播放'));
  }

  private log(message: string) {
    console.log(`[HLSPlayer] ${message}`);
  }

  private handleVideoError = (e: Event) => {
    const error = (e.target as HTMLVideoElement).error;
    this.log(`视频错误: ${error?.message}`);
  };

  private handleSeeking = async () => {
    const time = this.videoElement.currentTime;
    this.log(`Seeking to ${time}`);

    // 找到对应的片段
    const segmentDuration = 10; // 假设每个片段10秒
    const targetIndex = Math.floor(time / segmentDuration);

    if (targetIndex !== this.currentIndex) {
      this.currentIndex = targetIndex;
      await this.clearBuffer();
      await this.loadNextSegment();
    }
  };

  public async load(m3u8Url: string): Promise<void> {
    if (this.isDestroyed) {
      throw new Error('Player has been destroyed');
    }

    try {
      this.log(`开始加载 M3U8: ${m3u8Url}`);
      const response = await fetch(m3u8Url);
      const content = await response.text();
      this.currentPlaylist = this.parseM3U8(content, m3u8Url);
      this.log(`解析到 ${this.currentPlaylist.length} 个 MP4 文件`);

      if (this.currentPlaylist.length === 0) {
        throw new Error('未找到可播放的 MP4 文件');
      }

      this.currentIndex = 0;
      await this.initMSE();
    } catch (error) {
      this.log(`加载 M3U8 文件失败: ${error}`);
      throw error;
    }
  }

  private parseM3U8(content: string, baseUrl: string): string[] {
    const lines = content.split('\n');
    const mp4Urls: string[] = [];

    for (const line of lines) {
      if (line.trim() && !line.startsWith('#')) {
        const url = line.startsWith('http') ? line : new URL(line, baseUrl).href;
        mp4Urls.push(url);
        this.log(`找到 MP4: ${url}`);
      }
    }

    return mp4Urls;
  }

  private async initMSE(): Promise<void> {
    this.log('初始化 MSE');

    if (this.mediaSource) {
      if (this.mediaSource.readyState === 'open') {
        this.mediaSource.endOfStream();
      }
      URL.revokeObjectURL(this.videoElement.src);
      this.log('清理旧的 MediaSource');
    }

    this.mediaSource = new MediaSource();
    this.videoElement.src = URL.createObjectURL(this.mediaSource);
    this.pendingBuffers = [];
    this.isBuffering = false;

    return new Promise((resolve, reject) => {
      if (!this.mediaSource) return reject(new Error('MediaSource is null'));

      this.mediaSource.addEventListener('sourceopen', async () => {
        this.log('MediaSource 已打开');
        try {
          await this.initSourceBuffer();
          await this.loadFirstSegment();
          resolve();
        } catch (error) {
          reject(error);
        }
      });

      this.mediaSource.addEventListener('sourceended', () => {
        this.log('MediaSource 已结束');
      });

      this.mediaSource.addEventListener('error', (e) => {
        this.log(`MediaSource 错误: ${e}`);
        reject(e);
      });
    });
  }

  private async initSourceBuffer(): Promise<void> {
    if (!this.mediaSource) throw new Error('MediaSource is null');

    const codecConfigs = [
      'video/mp4; codecs="avc1.64001f"',
      'video/mp4; codecs="avc1.64001f,mp4a.40.2"',
      'video/mp4'
    ];

    for (const codec of codecConfigs) {
      try {
        if (MediaSource.isTypeSupported(codec)) {
          this.sourceBuffer = this.mediaSource.addSourceBuffer(codec);
          this.log(`成功创建 SourceBuffer，使用编解码器: ${codec}`);

          this.sourceBuffer.mode = 'sequence';
          this.sourceBuffer.addEventListener('updateend', this.handleUpdateEnd);
          this.sourceBuffer.addEventListener('error', (e) => {
            this.log(`SourceBuffer 错误: ${e}`);
          });

          return;
        }
      } catch (e) {
        this.log(`尝试编解码器 ${codec} 失败: ${e}`);
      }
    }

    throw new Error('无法创建支持的 SourceBuffer');
  }

  private async loadFirstSegment(): Promise<void> {
    this.log('等待第一个片段加载完成...');
    await this.loadNextSegment();

    return new Promise((resolve) => {
      const checkBuffer = () => {
        if (this.sourceBuffer && !this.sourceBuffer.updating && this.videoElement.buffered.length > 0) {
          this.log('首个片段缓冲完成');
          resolve();
        } else {
          setTimeout(checkBuffer, 100);
        }
      };
      checkBuffer();
    });
  }

  private async loadNextSegment(): Promise<void> {
    if (this.currentIndex >= this.currentPlaylist.length) {
      if (this.mediaSource?.readyState === 'open') {
        this.mediaSource.endOfStream();
        this.log('已到达播放列表末尾');
      }
      return;
    }

    try {
      await this.removeOldBuffers();

      this.log(`加载视频片段 ${this.currentIndex + 1}`);
      const response = await fetch(this.currentPlaylist[this.currentIndex]);
      if (!response.ok) {
        throw new Error(`HTTP error! status: ${response.status}`);
      }
      const buffer = await response.arrayBuffer();
      this.log(`视频片段 ${this.currentIndex + 1} 加载完成，大小: ${buffer.byteLength} 字节`);
      this.appendBuffer(buffer);

      // 预加载下一个片段
      if (this.currentIndex < this.currentPlaylist.length - 1 && this.pendingBuffers.length < 2) {
        this.currentIndex++;
        this.loadNextSegment();
      }
    } catch (error) {
      this.log(`加载视频片段失败: ${error}`);
      throw error;
    }
  }

  private async removeOldBuffers(): Promise<void> {
    if (!this.sourceBuffer || !this.videoElement.buffered.length) return;

    const currentTime = this.videoElement.currentTime;
    const buffered = this.videoElement.buffered;

    for (let i = 0; i < buffered.length; i++) {
      const start = buffered.start(i);
      const end = buffered.end(i);

      if (end - currentTime > this.maxBufferLength) {
        const removeEnd = currentTime - 1;
        if (removeEnd > start) {
          try {
            this.log(`清理缓冲区: ${start.toFixed(2)} - ${removeEnd.toFixed(2)}`);
            await new Promise<void>((resolve) => {
              if (!this.sourceBuffer) return resolve();

              this.sourceBuffer.remove(start, removeEnd);
              const onUpdate = () => {
                this.sourceBuffer?.removeEventListener('updateend', onUpdate);
                resolve();
              };
              this.sourceBuffer.addEventListener('updateend', onUpdate);
            });
          } catch (e) {
            this.log(`清理缓冲区失败: ${e}`);
          }
        }
      }
    }
  }

  private appendBuffer(buffer: ArrayBuffer): void {
    if (!this.sourceBuffer || this.sourceBuffer.updating || this.pendingBuffers.length > 0) {
      if (this.pendingBuffers.length < 3) {
        this.pendingBuffers.push(buffer);
        this.log('缓冲区正忙，将数据加入队列');
      } else {
        this.log('等待队列已满，丢弃数据');
      }
      return;
    }

    try {
      this.sourceBuffer.appendBuffer(buffer);
      this.isBuffering = true;
      this.log('添加数据到缓冲区');
    } catch (error) {
      if (error instanceof Error && error.name === 'QuotaExceededError') {
        this.log('缓冲区已满，将进行清理');
        this.pendingBuffers.push(buffer);
        this.removeOldBuffers();
      } else {
        this.log(`添加缓冲区失败: ${error}`);
        throw error;
      }
    }
  }

  private handleUpdateEnd = (): void => {
    this.isBuffering = false;
    this.log('缓冲区更新完成');

    if (this.pendingBuffers.length > 0) {
      const nextBuffer = this.pendingBuffers.shift();
      if (nextBuffer) {
        this.appendBuffer(nextBuffer);
      }
    }
  };

  private async clearBuffer(): Promise<void> {
    if (!this.sourceBuffer || !this.videoElement.buffered.length) return;

    try {
      const start = this.videoElement.buffered.start(0);
      const end = this.videoElement.buffered.end(this.videoElement.buffered.length - 1);

      await new Promise<void>((resolve) => {
        if (!this.sourceBuffer) return resolve();

        this.sourceBuffer.remove(start, end);
        const onUpdate = () => {
          this.sourceBuffer?.removeEventListener('updateend', onUpdate);
          resolve();
        };
        this.sourceBuffer.addEventListener('updateend', onUpdate);
      });

      this.pendingBuffers = [];
      this.isBuffering = false;
    } catch (e) {
      this.log(`清理缓冲区失败: ${e}`);
    }
  }

  public async play(): Promise<void> {
    try {
      await this.videoElement.play();
    } catch (error) {
      this.log(`播放失败: ${error}`);
      throw error;
    }
  }

  public pause(): void {
    this.videoElement.pause();
  }

  public seek(time: number): void {
    if (time < 0) time = 0;
    const duration = this.videoElement.duration;
    if (duration && time > duration) time = duration;

    this.videoElement.currentTime = time;
  }

  public setPlaybackRate(rate: number): void {
    if (rate <= 0) {
      throw new Error('Playback rate must be greater than 0');
    }
    this.videoElement.playbackRate = rate;
  }

  public destroy(): void {
    this.isDestroyed = true;
    this.pause();

    if (this.mediaSource?.readyState === 'open') {
      this.mediaSource.endOfStream();
    }

    if (this.videoElement.src) {
      URL.revokeObjectURL(this.videoElement.src);
    }

    this.videoElement.removeEventListener('error', this.handleVideoError);
    this.videoElement.removeEventListener('seeking', this.handleSeeking);

    this.sourceBuffer = null;
    this.mediaSource = null;
    this.currentPlaylist = [];
    this.pendingBuffers = [];
  }
} 