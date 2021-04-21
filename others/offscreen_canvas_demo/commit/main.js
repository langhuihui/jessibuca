var g_offscreen_canvas = null;
var g_render_worker = null;

function main() {
  g_offscreen_canvas = helper.GetCanvas("offscreen");

  let window_size = helper.GetWindowSizeInPx();
  g_offscreen_canvas.width = window_size.width;
  g_offscreen_canvas.height = window_size.height;

  g_render_worker = new Worker("../common/render.js");

  let offscreen = g_offscreen_canvas.transferControlToOffscreen();
  g_render_worker.postMessage(
    {name:"Init", mode:"commit", canvas:offscreen}, [offscreen]);
}