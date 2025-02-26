import { chromium, Page } from '@playwright/test';

async function setupLogging(page: Page) {
  // Log console messages with timestamps
  page.on('console', msg => {
    const timestamp = new Date().toISOString();
    console.log(`[${timestamp}] [Browser ${msg.type()}] ${msg.text()}`);
  });

  // Log page errors
  page.on('pageerror', err => {
    console.error(`[Browser Error] ${err.message}`);
  });

  // Log request failures
  page.on('requestfailed', request => {
    console.error(`[Network Error] ${request.url()} ${request.failure()?.errorText}`);
  });

  // Log network requests and responses
  page.on('request', request => {
    if (request.url().includes('.m3u8') || request.url().includes('.ts')) {
      console.log(`[Network Request] ${request.method()} ${request.url()}`);
      const headers = request.headers();
      if (Object.keys(headers).length > 0) {
        console.log('Headers:', JSON.stringify(headers, null, 2));
      }
    }
  });

  page.on('response', async response => {
    const url = response.url();
    if (url.includes('.m3u8') || url.includes('.ts')) {
      console.log(`[Network Response] ${url} - ${response.status()}`);
      const headers = response.headers();
      console.log('Response Headers:', JSON.stringify(headers, null, 2));

      // Log M3U8 content
      if (url.includes('.m3u8')) {
        try {
          const text = await response.text();
          console.log('[M3U8 Content]:', text);

          // Parse and analyze M3U8
          const segments = text.split('\n')
            .filter((line: string) => line.trim() && !line.startsWith('#'))
            .length;
          console.log(`[M3U8 Analysis] Found ${segments} segments`);
        } catch (err) {
          console.error('[M3U8 Parse Error]:', err);
        }
      }

      // Log TS segment info
      if (url.includes('.ts')) {
        console.log('[TS Segment] Received:', url);
        console.log('[TS Segment] Size:', headers['content-length']);
      }
    }
  });
}

async function debugHLS() {
  console.log('[Test] Starting HLS debug test...');

  const browser = await chromium.launch({
    headless: false,
    args: ['--autoplay-policy=no-user-gesture-required']
  });

  const context = await browser.newContext();
  const page = await context.newPage();

  await setupLogging(page);

  try {
    console.log('[Test] Navigating to page...');
    await page.goto('http://localhost:5173/');

    // Wait for page to load
    await page.waitForLoadState('networkidle');

    console.log('[Test] Clicking Connect button...');
    await page.click('button:has-text("Connect")');

    // Wait for initial setup and monitor the process
    console.log('[Test] Monitoring HLS streaming process...');

    // Monitor for 10 seconds
    for (let i = 0; i < 5; i++) {
      await page.waitForTimeout(2000);

      // Collect state
      const state = await page.evaluate(() => {
        const video = document.querySelector('video');
        const canvas = document.querySelector('canvas');

        // Get stats from display
        const stats = {
          audioTS: (window as any).display?.audioTS,
          videoTS: (window as any).display?.videoTS,
          videoDecodedFrames: (window as any).display?.videoDecodedFrames,
          audioDecodedFrames: (window as any).display?.audioDecodedFrames,
          videoDecodedFrameRate: (window as any).display?.videoDecodedFrameRate,
          audioDecodedFrameRate: (window as any).display?.audioDecodedFrameRate,
        };

        return {
          timestamp: new Date().toISOString(),
          videoElement: video ? {
            currentTime: video.currentTime,
            readyState: video.readyState,
            buffered: video.buffered.length > 0 ? {
              start: video.buffered.start(0),
              end: video.buffered.end(0)
            } : null,
            paused: video.paused,
            error: video.error?.message
          } : null,
          canvasElement: canvas ? {
            width: canvas.width,
            height: canvas.height,
            hasContext: !!canvas.getContext('2d')
          } : null,
          stats,
          demuxerState: (window as any).demuxerState,
          playerState: (window as any).playerState
        };
      });

      console.log(`[Test] State at ${i * 2}s:`, JSON.stringify(state, null, 2));

      // Check for errors or issues
      if (state.videoElement?.error) {
        console.error('[Test] Video Error:', state.videoElement.error);
      }

      // Verify decoding progress
      if (i > 0) {
        if (state.stats.videoDecodedFrames === 0) {
          console.warn('[Test] No video frames decoded yet');
        }
        if (state.stats.audioDecodedFrames === 0) {
          console.warn('[Test] No audio frames decoded yet');
        }
      }
    }

  } catch (error) {
    console.error('[Test] Test failed:', error);
  } finally {
    console.log('[Test] Cleaning up...');
    await browser.close();
  }
}

// Enable detailed error stacks
Error.stackTraceLimit = Infinity;

// Run test
debugHLS().catch(console.error); 