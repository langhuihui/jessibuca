const CDP = require('chrome-remote-interface');
const fs = require('fs');

async function clickM3U8Button() {
  console.log('Starting simplified CDP click script...');

  let client;
  try {
    // Connect to Chrome
    client = await CDP();
    const { Page, Runtime, Console } = client;

    // Enable domains
    await Promise.all([
      Page.enable(),
      Runtime.enable(),
      Console.enable()
    ]);

    // Set up console logger
    Console.messageAdded(({ message }) => {
      console.log(`Browser Console: ${message.text}`);
    });

    // Navigate to the page
    console.log('Navigating to http://localhost:5173/');
    await Page.navigate({ url: 'http://localhost:5173/' });
    await Page.loadEventFired();
    console.log('Page loaded');

    // Wait for page to render
    console.log('Waiting for 3 seconds for page to fully render...');
    await new Promise(resolve => setTimeout(resolve, 3000));

    // Take screenshot before clicking
    const beforeScreenshot = await Page.captureScreenshot();
    fs.writeFileSync('before-click.png', Buffer.from(beforeScreenshot.data, 'base64'));
    console.log('Saved screenshot before clicking to before-click.png');

    // Find the button with various selectors
    console.log('Attempting to find the M3U8 button...');
    const buttonSearch = await Runtime.evaluate({
      expression: `
        (() => {
          // First try by ID (most reliable if the ID has been added)
          let button = document.getElementById('load-m3u8-btn');
          if (button) return { found: true, type: 'by-id', text: button.textContent.trim() };
          
          // Second try by querySelector for a primary button with the right text
          button = document.querySelector('button.n-button--primary-type');
          if (button && button.textContent.includes('加载M3U8')) {
            return { found: true, type: 'by-class-and-text', text: button.textContent.trim() };
          }
          
          // Last try by text content
          button = Array.from(document.querySelectorAll('button')).find(
            b => b.textContent.includes('加载M3U8') || 
                 b.textContent.includes('load m3u8')
          );
          if (button) {
            return { found: true, type: 'by-text', text: button.textContent.trim() };
          }
          
          // If no button found, return list of all buttons
          return { 
            found: false, 
            buttons: Array.from(document.querySelectorAll('button')).map(
              b => ({ text: b.textContent.trim(), class: b.className })
            )
          };
        })()
      `,
      returnByValue: true
    });

    // Safe check on result structure
    if (!buttonSearch || !buttonSearch.result || typeof buttonSearch.result.value === 'undefined') {
      console.log('Failed to get button search result. Raw response:', JSON.stringify(buttonSearch));
      return;
    }

    const searchResult = buttonSearch.result.value;
    console.log('Button search result:', JSON.stringify(searchResult, null, 2));

    if (searchResult.found) {
      console.log(`Found button by ${searchResult.type} with text "${searchResult.text}". Clicking...`);

      // Click the button
      const clickResult = await Runtime.evaluate({
        expression: `
          (() => {
            try {
              // Find with the same strategy that succeeded
              let button;
              
              if ('${searchResult.type}' === 'by-id') {
                button = document.getElementById('load-m3u8-btn');
              } else if ('${searchResult.type}' === 'by-class-and-text') {
                button = document.querySelector('button.n-button--primary-type');
              } else {
                button = Array.from(document.querySelectorAll('button')).find(
                  b => b.textContent.includes('加载M3U8') || 
                       b.textContent.includes('load m3u8')
                );
              }
              
              if (button) {
                console.log('Clicking button:', button.textContent);
                button.click();
                return { success: true };
              } else {
                return { success: false, reason: 'Button no longer found' };
              }
            } catch (e) {
              return { success: false, error: e.toString() };
            }
          })()
        `,
        returnByValue: true
      });

      if (!clickResult || !clickResult.result || typeof clickResult.result.value === 'undefined') {
        console.log('Failed to get click result. Raw response:', JSON.stringify(clickResult));
        return;
      }

      console.log('Click result:', JSON.stringify(clickResult.result.value, null, 2));

      if (clickResult.result.value.success) {
        console.log('Button clicked successfully. Waiting 5 seconds to observe changes...');
        await new Promise(resolve => setTimeout(resolve, 5000));

        // Take after screenshot
        const afterScreenshot = await Page.captureScreenshot();
        fs.writeFileSync('after-click.png', Buffer.from(afterScreenshot.data, 'base64'));
        console.log('Saved screenshot after clicking to after-click.png');

        // Check for video element status
        const videoStatus = await Runtime.evaluate({
          expression: `
            (() => {
              const video = document.querySelector('video');
              if (!video) return { found: false };
              
              return {
                found: true,
                src: video.src,
                currentSrc: video.currentSrc,
                readyState: video.readyState,
                paused: video.paused,
                error: video.error ? video.error.message : null
              };
            })()
          `,
          returnByValue: true
        });

        console.log('Video status after click:', JSON.stringify(videoStatus.result.value, null, 2));

        // Look for logs in the UI
        const uiLogs = await Runtime.evaluate({
          expression: `
            (() => {
              try {
                // Get logs from the UI component
                const logs = [];
                document.querySelectorAll('.log-entry').forEach(entry => {
                  const type = entry.querySelector('.log-type')?.textContent || '';
                  const message = entry.querySelector('.log-message')?.textContent || '';
                  logs.push({ type, message });
                });
                return { logs };
              } catch (e) {
                return { error: e.toString() };
              }
            })()
          `,
          returnByValue: true
        });

        console.log('UI logs:', JSON.stringify(uiLogs.result.value, null, 2));
      } else {
        console.log('Failed to click button:', clickResult.result.value.reason || clickResult.result.value.error);
      }
    } else {
      console.log('Could not find the M3U8 button. All buttons:', JSON.stringify(searchResult.buttons, null, 2));
    }
  } catch (error) {
    console.error('Error in CDP script:', error);
  } finally {
    if (client) {
      await client.close();
    }
    console.log('CDP session ended');
  }
}

clickM3U8Button().catch(console.error); 