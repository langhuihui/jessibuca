var g_offscreen_canvas = null;
var g_compute_worker = null;
var g_render_worker = null;

const kBoids = 500;
const kBufferLength = kBoids * 5 * Float32Array.BYTES_PER_ELEMENT;

function main() {
  g_offscreen_canvas = helper.GetCanvas("offscreen");

  let window_size = helper.GetWindowSizeInPx();
  g_offscreen_canvas.width = window_size.width;
  g_offscreen_canvas.height = window_size.height;

  let sab = new SharedArrayBuffer(kBufferLength + Lock.NUMBYTES + Cond.NUMBYTES + 4);
  Lock.initialize(sab, kBufferLength);
  Cond.initialize(sab, kBufferLength + Lock.NUMBYTES);

  let offscreen = g_offscreen_canvas.transferControlToOffscreen();

  g_render_worker = new Worker("./sab_render.js");
  g_render_worker.postMessage(
      {name:"Init", canvas:offscreen, buffer:sab, boids:kBoids}, [offscreen]);

  g_compute_worker = new Worker("./sab_compute.js");
  g_compute_worker.postMessage(
      {name:"Init", width:window_size.width, height:window_size.height, buffer:sab, boids:kBoids});
}