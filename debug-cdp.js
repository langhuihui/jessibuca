const CDP = require('chrome-remote-interface');
const fs = require('fs');

async function captureBasicPageInfo() {
  console.log('Starting basic page capture...');

  let client;
  try {
    // Connect to Chrome
    client = await CDP();
    const { Page, Runtime } = client;

    // Enable domains
    await Promise.all([Page.enable(), Runtime.enable()]);

    // Navigate to page
    console.log('Navigating to http://localhost:5173/');
    await Page.navigate({ url: 'http://localhost:5173/' });
    await Page.loadEventFired();
    console.log('Page loaded');

    // Wait for any dynamic content
    await new Promise(resolve => setTimeout(resolve, 3000));

    // Capture HTML
    console.log('Capturing page HTML...');
    const htmlResult = await Runtime.evaluate({ expression: 'document.documentElement.outerHTML' });
    const html = htmlResult.result.value;
    fs.writeFileSync('page-content.html', html);
    console.log('Saved HTML to page-content.html');

    // Take screenshot
    console.log('Taking screenshot...');
    const screenshotResult = await Page.captureScreenshot();
    const screenshot = Buffer.from(screenshotResult.data, 'base64');
    fs.writeFileSync('screenshot.png', screenshot);
    console.log('Saved screenshot to screenshot.png');

    // Basic page analysis
    console.log('\nPerforming basic page analysis...');
    const analysisResult = await Runtime.evaluate({
      expression: `
        (() => {
          const result = {
            title: document.title,
            url: window.location.href,
            buttons: [],
            videos: [],
            iframes: [],
            elements: {}
          };
          
          // Count all elements
          const allElements = document.querySelectorAll('*');
          console.log('Total elements:', allElements.length);
          
          // Count by tag name
          const elementCounts = {};
          allElements.forEach(el => {
            const tag = el.tagName.toLowerCase();
            elementCounts[tag] = (elementCounts[tag] || 0) + 1;
          });
          result.elements = elementCounts;
          
          // Get all buttons
          document.querySelectorAll('button').forEach(btn => {
            result.buttons.push({
              text: btn.textContent.trim(),
              id: btn.id || null,
              class: btn.className || null,
              visible: btn.offsetParent !== null
            });
          });
          
          // Get all videos
          document.querySelectorAll('video').forEach(video => {
            result.videos.push({
              id: video.id || null,
              class: video.className || null,
              src: video.src || null,
              controls: video.controls,
              width: video.width,
              height: video.height
            });
          });
          
          // Get all iframes
          document.querySelectorAll('iframe').forEach(iframe => {
            result.iframes.push({
              id: iframe.id || null,
              src: iframe.src || null,
              width: iframe.width,
              height: iframe.height
            });
          });
          
          return result;
        })()
      `
    });

    if (analysisResult.result && analysisResult.result.value) {
      console.log('\nPage Analysis Result:');
      console.log(JSON.stringify(analysisResult.result.value, null, 2));
    } else {
      console.log('Failed to analyze page');
    }

    // Look for a "加载M3U8" button specifically
    console.log('\nLooking for 加载M3U8 button...');
    const buttonSearchResult = await Runtime.evaluate({
      expression: `
        (() => {
          // Direct search
          const allText = document.body.textContent;
          const hasM3U8Text = allText.includes('加载M3U8') || 
                             allText.includes('加载m3u8') || 
                             allText.includes('load M3U8');
          
          // Button search
          const buttons = Array.from(document.querySelectorAll('button'));
          const buttonTexts = buttons.map(b => b.textContent.trim());
          
          // Any element with load text
          const loadElements = Array.from(document.querySelectorAll('*')).filter(
            el => el.textContent.includes('加载') || 
                 el.textContent.includes('load')
          );
          
          return {
            hasM3U8TextInBody: hasM3U8Text,
            allButtonTexts: buttonTexts,
            loadElementsCount: loadElements.length,
            loadElementTexts: loadElements.map(el => ({
              tag: el.tagName.toLowerCase(),
              text: el.textContent.trim().substring(0, 50)
            }))
          };
        })()
      `
    });

    if (buttonSearchResult.result && buttonSearchResult.result.value) {
      console.log('\nLoad Button Search Result:');
      console.log(JSON.stringify(buttonSearchResult.result.value, null, 2));
    } else {
      console.log('Failed to search for load button');
    }

  } catch (err) {
    console.error('Error during capture:', err);
  } finally {
    if (client) {
      await client.close();
    }
  }

  console.log('\nBasic page capture completed');
}

async function captureAndClickM3U8Button() {
  console.log('Starting CDP session to load M3U8...');

  let client;
  try {
    // Connect to Chrome
    client = await CDP();
    const { Page, Runtime, Network, Console } = client;

    // Enable necessary domains
    await Promise.all([
      Page.enable(),
      Runtime.enable(),
      Network.enable(),
      Console.enable()
    ]);

    // Collect console logs
    const logs = [];
    Console.messageAdded(({ message }) => {
      const text = message.text;
      logs.push(text);
      console.log(`[Browser Console] ${text}`);
    });

    // Navigate to page
    console.log('Navigating to http://localhost:5173/');
    await Page.navigate({ url: 'http://localhost:5173/' });
    await Page.loadEventFired();
    console.log('Page loaded');

    // Wait for page to fully render
    await new Promise(resolve => setTimeout(resolve, 3000));

    // Look for and click the load M3U8 button using its id
    console.log('Looking for the load-m3u8-btn button...');
    const findButtonResult = await Runtime.evaluate({
      expression: `
        (() => {
          // Try to find button by ID first (most reliable)
          const buttonById = document.getElementById('load-m3u8-btn');
          if (buttonById) {
            return { found: true, method: 'by-id', element: 'button#load-m3u8-btn' };
          }
          
          // Fallback to other methods if ID not found
          const loadButton = Array.from(document.querySelectorAll('button')).find(
            b => b.textContent.includes('加载M3U8')
          );
          
          if (loadButton) {
            return { found: true, method: 'by-text', element: 'button:contains(加载M3U8)' };
          }
          
          return { 
            found: false, 
            allButtons: Array.from(document.querySelectorAll('button')).map(b => b.textContent.trim())
          };
        })()
      `
    });

    console.log('Button search result:', JSON.stringify(findButtonResult.result.value, null, 2));

    if (findButtonResult.result.value.found) {
      console.log(`Found the button using ${findButtonResult.result.value.method}, clicking...`);

      // Click the button
      const clickResult = await Runtime.evaluate({
        expression: `
          (() => {
            try {
              const buttonById = document.getElementById('load-m3u8-btn');
              if (buttonById) {
                buttonById.click();
                return { success: true, method: 'by-id' };
              }
              
              const loadButton = Array.from(document.querySelectorAll('button')).find(
                b => b.textContent.includes('加载M3U8')
              );
              
              if (loadButton) {
                loadButton.click();
                return { success: true, method: 'by-text' };
              }
              
              return { success: false, error: 'Button not found on click attempt' };
            } catch (e) {
              return { success: false, error: e.toString() };
            }
          })()
        `
      });

      console.log('Button click result:', JSON.stringify(clickResult.result.value, null, 2));

      if (clickResult.result.value.success) {
        console.log('Button clicked successfully. Waiting for video to load (10s)...');
        await new Promise(resolve => setTimeout(resolve, 10000));

        // Check video element status
        const videoStatus = await Runtime.evaluate({
          expression: `
            (() => {
              const videoEl = document.querySelector('video');
              if (!videoEl) return { found: false };
              
              return {
                found: true,
                src: videoEl.src,
                currentSrc: videoEl.currentSrc,
                readyState: videoEl.readyState,
                networkState: videoEl.networkState,
                paused: videoEl.paused,
                ended: videoEl.ended,
                duration: videoEl.duration,
                error: videoEl.error ? videoEl.error.message : null,
                videoWidth: videoEl.videoWidth,
                videoHeight: videoEl.videoHeight
              };
            })()
          `
        });

        console.log('\nVideo element status:');
        console.log(JSON.stringify(videoStatus.result.value, null, 2));

        // Check MediaSource and SourceBuffer status
        const mseStatus = await Runtime.evaluate({
          expression: `
            (() => {
              try {
                const videoEl = document.querySelector('video');
                if (!videoEl) return { videoFound: false };
                
                return {
                  videoFound: true,
                  usingMediaSource: videoEl.src.startsWith('blob:'),
                  src: videoEl.src,
                  seeking: videoEl.seeking,
                  error: videoEl.error ? videoEl.error.message : null,
                  // Check if demuxer exists and is initialized
                  appState: (() => {
                    try {
                      // Try to get the demuxer instance from Vue app
                      if (window.__VUE_DEVTOOLS_GLOBAL_HOOK__) {
                        return 'Vue DevTools detected, may be able to access component state';
                      }
                      return 'Unable to access Vue component state directly';
                    } catch (e) {
                      return 'Error accessing Vue state: ' + e.toString();
                    }
                  })()
                };
              } catch (e) {
                return { error: e.toString() };
              }
            })()
          `
        });

        console.log('\nMediaSource status:');
        console.log(JSON.stringify(mseStatus.result.value, null, 2));

        // Check network requests for m3u8 and media segments
        const networkRequests = await Runtime.evaluate({
          expression: `
            (() => {
              try {
                const resources = performance.getEntriesByType('resource');
                return {
                  m3u8Requests: resources.filter(r => r.name.includes('.m3u8')).map(r => ({
                    url: r.name,
                    duration: r.duration,
                    startTime: r.startTime,
                    responseEnd: r.responseEnd
                  })),
                  segmentRequests: resources.filter(r => 
                    r.name.includes('.ts') || 
                    r.name.includes('.mp4') ||
                    r.name.includes('.m4s')
                  ).map(r => ({
                    url: r.name,
                    duration: r.duration,
                    startTime: r.startTime,
                    responseEnd: r.responseEnd
                  }))
                };
              } catch (e) {
                return { error: e.toString() };
              }
            })()
          `
        });

        console.log('\nNetwork requests for video resources:');
        console.log(JSON.stringify(networkRequests.result.value, null, 2));

        // Take a screenshot after clicking
        const screenshot = await Page.captureScreenshot();
        fs.writeFileSync('after-click-screenshot.png', Buffer.from(screenshot.data, 'base64'));
        console.log('Screenshot saved to after-click-screenshot.png');

        // Get any logs from the UI component
        const uiLogs = await Runtime.evaluate({
          expression: `
            (() => {
              // Try to find logs in the DOM
              const logEntries = Array.from(document.querySelectorAll('.log-entry'));
              if (logEntries.length > 0) {
                return logEntries.map(entry => {
                  const timestamp = entry.querySelector('.log-timestamp')?.textContent || '';
                  const type = entry.querySelector('.log-type')?.textContent || '';
                  const message = entry.querySelector('.log-message')?.textContent || '';
                  return { timestamp, type, message };
                });
              }
              return { error: 'No log entries found in DOM' };
            })()
          `
        });

        console.log('\nUI Logs:');
        console.log(JSON.stringify(uiLogs.result.value, null, 2));
      } else {
        console.log('Failed to click button:', clickResult.result.value.error);
      }
    } else {
      console.log('Could not find the button. Available buttons:',
        findButtonResult.result.value.allButtons);
    }

    console.log('\nCapture complete. Check the logs and screenshots for details.');
  } catch (err) {
    console.error('Error during capture:', err);
  } finally {
    if (client) {
      await client.close();
    }
  }
}

captureBasicPageInfo().catch(err => {
  console.error('Error:', err);
});

captureAndClickM3U8Button().catch(err => {
  console.error('Error in main function:', err);
}); 