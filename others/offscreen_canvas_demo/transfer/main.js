var g_bitmap_canvas = null;
var g_2d_canvas = null;
var g_render_worker = null;
var g_angle = 30;

function main() {
  g_bitmap_canvas = helper.GetCanvas("bitmap");
  // g_2d_canvas = helper.GetCanvas("2d");

  let window_size = helper.GetWindowSizeInPx();
  g_bitmap_canvas.width = window_size.width;
  g_bitmap_canvas.height = window_size.height / 2;

  // g_2d_canvas.width = window_size.width;
  // g_2d_canvas.height = window_size.height / 2;

  g_render_worker = new Worker("../common/render.js");
  g_render_worker.onmessage = function(msg) {
    console.log('Woker post:' + msg.data.name);

    if (msg.data.name === "TransferBuffer") {
      GetTransferBuffer(msg.data.buffer);
    }
  }

  g_render_worker.postMessage(
    {name:"Init", mode:"transfer",
      width:g_bitmap_canvas.width, height: g_bitmap_canvas.height});
}

// 还是有数据传递。
function GetTransferBuffer(buffer) {

  // // 但我们使用 OffscreenCanvas，通过 2D/3D 进行绘制时，就好像我们有一块画板，上面有一些画纸，我们可以在画纸上作画；
  // let context_2d = g_2d_canvas.getContext("2d");
  // context_2d.clearRect(0, 0, g_2d_canvas.width, g_2d_canvas.height);
  // context_2d.save();
  // context_2d.translate(g_bitmap_canvas.width / 2, g_bitmap_canvas.height / 2);
  // context_2d.rotate(g_angle * Math.PI / 180);
  // context_2d.scale(0.5, 0.5);
  // context_2d.translate(-g_bitmap_canvas.width / 2, -g_bitmap_canvas.height / 2);
  // // 把 ImageBitmap 作为 Image 绘制在一个 2D Canvas 上，就好像我们对已经绘制好的图画在新的画纸上进行临摹；
  // context_2d.drawImage(buffer, 0, 0);
  // context_2d.restore();
  //
  // g_angle += 15;
  // if (g_angle > 360)
  //   g_angle = 0;




  let bitmap_context = g_bitmap_canvas.getContext("bitmaprenderer");
  // 把 ImageBitmap 通过 ImageBitmapRenderingContext.transferFromImageBitmap Transfer 给 Bitmap Canvas，就好像我们把画纸放入一个画框里挂在墙上显示；
  bitmap_context.transferFromImageBitmap(buffer);

//   简单的说 ImageBitmap Transfer 语义实现了 Zero Copy 的所有权转移，不需要对缓冲区进行拷贝，性能更高，但是也限制了显示的方式，
//   而临摹意味着我们可以对临摹的副本进行旋转，缩放，位移等等，还可以在上面再绘制其它内容。
//   另外 ImageBitmap Transfer 之后所有权就发生了转移，比如 Transfer Demo 的例程调换一下两个 Canvas 的绘制顺序就会报错，
//   这是因为 Transfer 之后，原来的缓冲区引用已经被置空变成一个空引用。
}