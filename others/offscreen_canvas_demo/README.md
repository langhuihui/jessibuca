# Offscreen Canvas Demo

## Transfer Demo

1. Main thread lanuch worker thread, then ask initialize;
2. Worker create OffscreenCanvas;
3. Worker get WebGL Context of OffscreenCanvas and render;
4. Worker get the buffer of OffscreenCanvas (ImageBitmap), and transfer back to main thread;
5. Main thread receive the buffer and 'draw' it to two different Canvas, One Canvas use CanvasRenderingContext2D，other use ImageBitmapRenderingContext;
6. Repeat 3 ~ 5;

## Commit Demo

1. Main thread get Canvas from current DOM and generate an OffscreenCanvas from that Canvas;
2. Main thread launch worker thread, then ask initialize, transfer OffscreenCanvas as parameter; 
3. Worker receive OffscreenCanvas and initialize;
4. Worker get WebGL Context of OffscreenCanvas and render;
5. Worker commit when render complete, and wait the callback;
6. Worker receive callback then repeat 4 ~ 6;

## Transfer Demo

1. 主线程启动 Worker 线程，并请求初始化；
2. Worker 线程创建 OffscreenCanvas；
3. Worker 线程获取 OffscreenCanvas 的 WebGL Context 并进行绘制；
4. Worker 线程获取 OffscreenCanvas 的缓冲区（ImageBitmap），然后 Transfer 回主线程；
5. 主线程将 Worker 线程回传的缓冲区分别绘制在两个不同的 Canvas 上，一个 Canvas 使用 CanvasRenderingContext2D，一个 Canvas 使用 ImageBitmapRenderingContext；
6. 3 ~ 5 重复运行；

## Commit Demo

1. 主线程从当前 DOM 树中的 Canvas 元素生成 OffscreenCanvas；
2. 主线程启动 Worker 线程并初始化，OffscreenCanvas 作为初始化的参数被 Transfer；
3. Worker 线程接收 OffscreenCanvas 后完成初始化；
4. Worker 线程使用 WebGL 对 OffscreenCanvas 进行绘制；
5. Worker 线程绘制完成后 Commit，然后等待浏览器的回调；
6. Worker 线程接收到到浏览器的回调后继续绘制下一帧，重复 4 ~ 6；
