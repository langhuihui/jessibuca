import { Connection } from 'jv4-connection/src/base';
import { WebSocketConnection } from 'jv4-connection/src/ws';
import { WebRTCConnection } from 'jv4-connection/src/webrtc';
import { HttpConnection } from 'jv4-connection/src/http';
import { getURLType } from 'jv4-connection/src';
import { FlvDemuxer } from 'jv4-demuxer/src/flv';
import { PSDemuxer } from 'jv4-demuxer/src/ps';
import { BaseDemuxer, DemuxEvent, DemuxMode } from 'jv4-demuxer/src/base';
import { VideoDecoder as SoftwareVideoDecoder } from 'jv4-decoder/src/video_decoder';
import { AudioDecoder as SoftwareAudioDecoder } from 'jv4-decoder/src/audio_decoder';

export interface JessibucaPlayerOptions {
  container?: HTMLElement;
  videoElement?: HTMLVideoElement;
  protocol?: 'ws-flv' | 'webrtc' | 'http-flv' | 'auto';
  demuxerType?: 'flv' | 'ps' | 'auto';
  useHardwareDecoder?: boolean;
  debug?: boolean;
  autoPlay?: boolean;
  reconnect?: boolean;
  reconnectDelay?: number;
  bufferTime?: number;
}

/**
 * General-purpose Jessibuca Player that supports ws-flv, webrtc, and http-flv protocols
 */
export class GeneralPlayer {
  private connection?: Connection;
  private demuxer?: BaseDemuxer;
  private videoDecoder?: SoftwareVideoDecoder;
  private audioDecoder?: SoftwareAudioDecoder;
  private videoElement: HTMLVideoElement;
  private options: Required<JessibucaPlayerOptions>;
  private url?: string;
  private _isPlaying = false;
  private isPaused = false;

  constructor(
    videoElement: HTMLVideoElement,
    options: JessibucaPlayerOptions = {}
  ) {
    this.videoElement = videoElement;
    this.options = {
      container: options.container || videoElement.parentElement!,
      videoElement: videoElement,
      protocol: options.protocol || 'auto',
      demuxerType: options.demuxerType || 'auto',
      useHardwareDecoder: options.useHardwareDecoder !== false,
      debug: options.debug || false,
      autoPlay: options.autoPlay || false,
      reconnect: options.reconnect !== false,
      reconnectDelay: options.reconnectDelay || 3000,
      bufferTime: options.bufferTime || 1000,
    };
  }

  /**
   * Load and connect to a stream
   */
  async load(url: string): Promise<void> {
    this.url = url;
    await this.connect();
  }

  /**
   * Connect to the stream
   */
  private async connect(): Promise<void> {
    if (!this.url) {
      throw new Error('No URL specified');
    }

    try {
      // Determine protocol type
      const urlType = getURLType(this.url);
      const protocol = this.options.protocol === 'auto' 
        ? this.detectProtocol(urlType)
        : this.options.protocol;

      this.log(`Connecting to ${this.url} using protocol: ${protocol}`);

      // Create connection based on protocol
      switch (urlType) {
        case 'ws':
          this.connection = new WebSocketConnection(this.url);
          break;
        case 'http':
          this.connection = new HttpConnection(this.url);
          break;
        case 'webrtc':
          this.connection = new WebRTCConnection(this.url);
          break;
        default:
          throw new Error(`Unsupported URL type: ${urlType}`);
      }

      // Setup connection event listeners
      this.setupConnectionListeners();

      // Connect
      await this.connection.connect();

      // Create demuxer based on protocol
      const demuxerType = this.options.demuxerType === 'auto'
        ? this.detectDemuxer(protocol)
        : this.options.demuxerType;

      this.log(`Creating ${demuxerType} demuxer`);

      switch (demuxerType) {
        case 'flv':
          this.demuxer = new FlvDemuxer(this.connection, DemuxMode.PULL);
          break;
        case 'ps':
          this.demuxer = new PSDemuxer(this.connection, DemuxMode.PULL);
          break;
        default:
          throw new Error(`Unsupported demuxer type: ${demuxerType}`);
      }

      // Setup demuxer event listeners
      this.setupDemuxerListeners();

      // Start playback if autoPlay is enabled
      if (this.options.autoPlay) {
        await this.play();
      }

      this.log('Connected successfully');
    } catch (error) {
      this.log(`Connection error: ${error}`, 'error');
      throw error;
    }
  }

  /**
   * Detect protocol from URL type
   */
  private detectProtocol(urlType: string): 'ws-flv' | 'webrtc' | 'http-flv' {
    switch (urlType) {
      case 'ws':
        return 'ws-flv';
      case 'http':
        return 'http-flv';
      case 'webrtc':
        return 'webrtc';
      default:
        return 'ws-flv';
    }
  }

  /**
   * Detect demuxer type from protocol
   */
  private detectDemuxer(protocol: string): 'flv' | 'ps' {
    // For most protocols, FLV is the default
    // PS is typically used for GB28181
    return protocol.includes('flv') ? 'flv' : 'flv';
  }

  /**
   * Setup connection event listeners
   */
  private setupConnectionListeners(): void {
    if (!this.connection) return;

    this.connection.on('connecting', () => {
      this.log('Connecting...');
    });

    this.connection.on('connected', () => {
      this.log('Connected');
    });

    this.connection.on('disconnected', () => {
      this.log('Disconnected');
      if (this.options.reconnect) {
        setTimeout(() => {
          this.log('Attempting to reconnect...');
          this.connect().catch(err => {
            this.log(`Reconnection failed: ${err}`, 'error');
          });
        }, this.options.reconnectDelay);
      }
    });

    this.connection.on('error', (error: Error) => {
      this.log(`Connection error: ${error.message}`, 'error');
    });
  }

  /**
   * Setup demuxer event listeners
   */
  private setupDemuxerListeners(): void {
    if (!this.demuxer) return;

    this.demuxer.on(DemuxEvent.VIDEO_ENCODER_CONFIG_CHANGED, (config: VideoDecoderConfig) => {
      this.log('Video encoder config changed');
      this.setupVideoDecoder(config);
    });

    this.demuxer.on(DemuxEvent.AUDIO_ENCODER_CONFIG_CHANGED, (config: AudioDecoderConfig) => {
      this.log('Audio encoder config changed');
      this.setupAudioDecoder(config);
    });
  }

  /**
   * Setup video decoder
   */
  private setupVideoDecoder(config: VideoDecoderConfig): void {
    this.log('Setting up video decoder');
    
    if (this.options.useHardwareDecoder && 'VideoDecoder' in window) {
      // Use hardware decoder if available
      this.setupHardwareVideoDecoder(config);
    } else {
      // Fallback to software decoder
      this.videoDecoder = new SoftwareVideoDecoder();
    }
  }

  /**
   * Setup hardware video decoder
   */
  private setupHardwareVideoDecoder(config: VideoDecoderConfig): void {
    // Implementation for hardware decoding would go here
    // This is a placeholder
    this.log('Hardware video decoder not fully implemented yet');
  }

  /**
   * Setup audio decoder
   */
  private setupAudioDecoder(config: AudioDecoderConfig): void {
    this.log('Setting up audio decoder');
    
    if (this.options.useHardwareDecoder && 'AudioDecoder' in window) {
      // Use hardware decoder if available
      this.setupHardwareAudioDecoder(config);
    } else {
      // Fallback to software decoder
      this.audioDecoder = new SoftwareAudioDecoder();
    }
  }

  /**
   * Setup hardware audio decoder
   */
  private setupHardwareAudioDecoder(config: AudioDecoderConfig): void {
    // Implementation for hardware decoding would go here
    // This is a placeholder
    this.log('Hardware audio decoder not fully implemented yet');
  }

  /**
   * Play the video
   */
  async play(): Promise<void> {
    if (!this.videoElement) {
      throw new Error('No video element');
    }

    try {
      await this.videoElement.play();
      this._isPlaying = true;
      this.isPaused = false;
      this.log('Playing');
    } catch (error) {
      this.log(`Play error: ${error}`, 'error');
      throw error;
    }
  }

  /**
   * Pause the video
   */
  pause(): void {
    if (!this.videoElement) {
      throw new Error('No video element');
    }

    this.videoElement.pause();
    this._isPlaying = false;
    this.isPaused = true;
    this.log('Paused');
  }

  /**
   * Stop and disconnect
   */
  stop(): void {
    this.log('Stopping');
    
    if (this.videoElement) {
      this.videoElement.pause();
      this.videoElement.src = '';
    }

    this._isPlaying = false;
    this.isPaused = false;

    this.disconnect();
  }

  /**
   * Disconnect from the stream
   */
  private disconnect(): void {
    if (this.demuxer) {
      // Clean up demuxer
      this.demuxer = undefined;
    }

    if (this.connection) {
      this.connection.close();
      this.connection = undefined;
    }

    if (this.videoDecoder) {
      this.videoDecoder = undefined;
    }

    if (this.audioDecoder) {
      this.audioDecoder = undefined;
    }

    this.log('Disconnected');
  }

  /**
   * Destroy the player and clean up resources
   */
  destroy(): void {
    this.stop();
    this.log('Destroyed');
  }

  /**
   * Check if playing
   */
  isPlaying(): boolean {
    return this._isPlaying && !this.isPaused;
  }

  /**
   * Get current time
   */
  getCurrentTime(): number {
    return this.videoElement?.currentTime || 0;
  }

  /**
   * Get duration
   */
  getDuration(): number {
    return this.videoElement?.duration || 0;
  }

  /**
   * Set volume
   */
  setVolume(volume: number): void {
    if (this.videoElement) {
      this.videoElement.volume = Math.max(0, Math.min(1, volume));
    }
  }

  /**
   * Get volume
   */
  getVolume(): number {
    return this.videoElement?.volume || 1;
  }

  /**
   * Mute
   */
  mute(): void {
    if (this.videoElement) {
      this.videoElement.muted = true;
    }
  }

  /**
   * Unmute
   */
  unmute(): void {
    if (this.videoElement) {
      this.videoElement.muted = false;
    }
  }

  /**
   * Check if muted
   */
  isMuted(): boolean {
    return this.videoElement?.muted || false;
  }

  /**
   * Set playback rate
   */
  setPlaybackRate(rate: number): void {
    if (this.videoElement) {
      this.videoElement.playbackRate = rate;
    }
  }

  /**
   * Get playback rate
   */
  getPlaybackRate(): number {
    return this.videoElement?.playbackRate || 1;
  }

  /**
   * Seek to specific time (not supported for live streams)
   */
  seek(time: number): void {
    if (this.videoElement) {
      this.videoElement.currentTime = time;
    }
  }

  /**
   * Set time ranges (for HLS compatibility, not used in general player)
   */
  setTimeRanges(ranges: any[]): void {
    // Not applicable for live streaming protocols
    this.log('setTimeRanges is not supported for live streaming protocols', 'debug');
  }

  /**
   * Log message
   */
  private log(message: string, level: 'info' | 'error' | 'debug' = 'info'): void {
    if (!this.options.debug && level === 'debug') {
      return;
    }

    const prefix = '[GeneralPlayer]';
    switch (level) {
      case 'error':
        console.error(`${prefix} ${message}`);
        break;
      case 'debug':
        console.debug(`${prefix} ${message}`);
        break;
      default:
        console.log(`${prefix} ${message}`);
    }
  }
}
