'use strict';

importScripts("../common/helper.js");
importScripts("../common/cuon-utils.js");
importScripts("../common/webgl-utils.js");
importScripts("../common/webgl-debug.js");
importScripts("../common/lock.js");

var gl = null;
var canvas = null;
var meter = new helper.FPSMeter();
var drawCount = 0;
var drawEnabled = true;
var firstDraw = true;

var num_boids = 0;
var lock_compute;
var lock_render;
var cond_compute;
var cond_render;

if (helper.IsWorkerEnv()) {
  self.onmessage = function(msg) {
    console.log('Window post to render worker:' + msg.data.name);

    if (msg.data.name === "Init") {
      Init(msg.data);
    }
  }
}

function Init(data) {
  num_boids = data.boids;
  canvas = data.canvas;
  console.log("Render init canvas size:" + canvas.width + "x" + canvas.height);

  let sab = data.buffer;
  let lock_offset = data.boids * 5 * Float32Array.BYTES_PER_ELEMENT;
  lock_compute = new Lock(sab, lock_offset, "Render");
  cond_compute = new Cond(lock_compute, lock_offset + Lock.NUMBYTES);

  if (canvas && canvas.getContext) {
    //setup page
    // Get the rendering context for WebGL
    gl = getWebGLContext(canvas);
    if (!gl) {
      console.log('Failed to get the rendering context for WebGL');
      return;
    }

    // Initialize shaders
    if (!initShaders(gl, VSHADER_SOURCE, FSHADER_SOURCE)) {
      console.log('Failed to intialize shaders.');
      return;
    }

    // // Get the storage location of a_Position
    a_Position = gl.getAttribLocation(gl.program, 'a_Position');
    if (a_Position < 0) {
      console.log('Failed to get the storage location of a_Position');
      return;
    }

    // Get the storage location of a_Color
    a_Color = gl.getAttribLocation(gl.program, 'a_Color');
    if (a_Color < 0) {
      console.log('Failed to get the storage location of a_Color');
      return;
    }

    // Specify the color for clearing <canvas>
    gl.clearColor(0.2, 0.2, 0.2, 1);
    
    // Allocate buffer for position and color
    verticesColors = new Float32Array(sab, 0, data.boids * 5);

    // Create a buffer object
    vertexColorBuffer = gl.createBuffer();
    if (!vertexColorBuffer) {
      console.log('Failed to create the buffer object');
      return false;
    }

    let compute_data = new Int32Array(sab, 
      lock_offset + Lock.NUMBYTES + Cond.NUMBYTES, 1);
  
    function addComputedData() {
      Atomics.store(compute_data, 0, 1);
    }
  
    function useComputedData() {
      Atomics.store(compute_data, 0, 0);
    }
  
    function hasComputedData() {
      return Atomics.load(compute_data, 0) != 0;
    }

    function renderloop() {
      lock_compute.lock();
      // Wait computer to compute new data
      while (!hasComputedData())
        cond_compute.wait();
      // Upload data
      uploadVertexBuffers(gl, firstDraw);
      useComputedData();
      // Wake computer to compute next
      cond_compute.wakeOne();
      lock_compute.unlock();

      // Render buffer
      render();

      // fps
      let result = meter.update();
      if (result.framerate > 0) {
        console.log("WebGL Offscreen SharedArrayBuffer framerate:" + result.framerate);
      }

      // Wait next begin frame to loop
      gl.commit().then(renderloop);
    }

    renderloop();
  }
}

// Vertex shader program
var VSHADER_SOURCE =
  'attribute vec4 a_Position;\n' +
  'attribute vec4 a_Color;\n' +
  'varying vec4 v_Color;\n' +
  'void main() {\n' +
  '  gl_Position = a_Position;\n' +
  '  gl_PointSize = 10.0;\n' +
  '  v_Color = a_Color;\n' +
  '}\n';

// Fragment shader program
var FSHADER_SOURCE =
  '#ifdef GL_ES\n' +
  'precision mediump float;\n' +
  '#endif\n' +
  'varying vec4 v_Color;\n' +
  'void main() {\n' +
  '  gl_FragColor = v_Color;\n' +
  '}\n';

var a_Position;
var a_Color;
var verticesColors;
var vertexColorBuffer;

function uploadVertexBuffers(gl, first) {
  // Bind the buffer object to target
  if (first) {
    gl.bindBuffer(gl.ARRAY_BUFFER, vertexColorBuffer);
  }

  gl.bufferData(gl.ARRAY_BUFFER, verticesColors, gl.DYNAMIC_DRAW);

  if (first) {
    let FSIZE = verticesColors.BYTES_PER_ELEMENT;
    gl.vertexAttribPointer(a_Position, 2, gl.FLOAT, false, FSIZE * 5, 0);
    gl.enableVertexAttribArray(a_Position);  // Enable the assignment of the buffer object

    gl.vertexAttribPointer(a_Color, 3, gl.FLOAT, false, FSIZE * 5, FSIZE * 2);
    gl.enableVertexAttribArray(a_Color);  // Enable the assignment of the buffer object
  }

  // Unbind the buffer object
  // gl.bindBuffer(gl.ARRAY_BUFFER, null);
}

function render() {
  if (drawEnabled){
    // Clear <canvas>
    gl.clear(gl.COLOR_BUFFER_BIT);

    // Purposely to slow it down
    let points_per_draw = 10;
    for (let i = 0; i < num_boids / points_per_draw; ++i)
      gl.drawArrays(gl.POINTS, i * points_per_draw, points_per_draw);

    firstDraw = false;
  }
}