/// <reference types="vite/client" />

declare module '*.vue' {
  import type { DefineComponent } from 'vue';
  const component: DefineComponent<{}, {}, any>;
  export default component;
}

// 声明 HLS Video Web Component
declare global {
  namespace JSX {
    interface IntrinsicElements {
      'hls-video': {
        src?: string;
        controls?: boolean;
        autoplay?: boolean;
        muted?: boolean;
        loop?: boolean;
        poster?: string;
        width?: string | number;
        height?: string | number;
        preload?: string;
        crossorigin?: string;
        playsinline?: boolean;
        volume?: string | number;
        'playback-rate'?: string | number;
        'current-time'?: string | number;
        'quality-level'?: string | number;
        debug?: boolean;
        'low-latency'?: boolean;
        'max-buffer-length'?: string | number;
      };
    }
  }
}
