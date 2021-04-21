'use strict';

importScripts("helper.js");
importScripts("Vector3D.js");
importScripts("Boid.js");
importScripts("cuon-utils.js");
importScripts("webgl-utils.js");
importScripts("webgl-debug.js");

// Basically RPC.
var STAGE = { width:1080, height:1920 };

var gl = null;
var canvas = null;
var meter = new helper.FPSMeter();
var drawCount = 0;

if (helper.IsWorkerEnv()) {
  self.onmessage = function(msg) {
    console.log('Window post:' + msg.data.name + ", mode:" + msg.data.mode);

    if (msg.data.name === "Init") {
      Init(msg.data.mode, msg.data);
    }
  }
}

function TransferBuffer() {
  //ImageBitmap 主要是用来封装一块 GPU 缓冲区，可以被 GPU 读写，并且实现了 Transferable 的接口，可以在不同线程之间 Transfer。
  // 跟 ImageData 不一样，ImageBitmap 并没有提供 JavaScipt API 供 CPU 进行读写，
  // 这是因为使用 CPU 读写 GPU 缓冲区的成本非常高，需要拷贝到临时缓冲区进行读写然后再写回。
  // 这也是为什么规范的制定者没有扩展 ImageData，而是提供了一个新的 ImageBitmap 的缘故。
  let image_bitmap = canvas.transferToImageBitmap();
  // ImageBitmap 可以被当做普通的 Image 绘制在一个 2D Canvas 上，也可以通过 ImageBitmapRenderingContext Transfer 到一个 Bitmap Canvas
  // ，我们通过举例来说明这两种方式的区别：
  postMessage({name:"TransferBuffer", buffer:image_bitmap},
    [image_bitmap]);
}

function Init(mode, data) {
  if (mode === "transfer")
    canvas = new OffscreenCanvas(data.width, data.height);
  else if (mode === "commit")
    canvas = data.canvas;

  console.log("Render init canvas size:" + canvas.width + "x" + canvas.height);

  STAGE.width = canvas.width;
  STAGE.height = canvas.height;

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

    //initialize test variables
    createBoids();

    // Allocate buffer for position and color
    verticesColors = new Float32Array(boids.length * 5);

    // Create a buffer object
    vertexColorBuffer = gl.createBuffer();
    if (!vertexColorBuffer) {
      console.log('Failed to create the buffer object');
      return false;
    }

    if (mode === "transfer") {
      setInterval(() => {
        // Render buffer first
        render();
        // Transfer render buffer back to browser context
        TransferBuffer();
      }, 200);
    } else if (mode === "commit") {
      function renderloop() {
        // Render buffer first
        render();

        // fps
        let result = meter.update();
        if (result.framerate > 0) {
          console.log("WebGL Offscreen framerate:" + result.framerate);
        }

        // Wait next begin frame to loop
        // chrome 浏览器不支持。
        gl.commit().then(renderloop);
      }

      renderloop();
    }
  }
}

var config = {
  minForce:3,
  maxForce:6,
  minSpeed:6,
  maxSpeed:12,
  minWanderDistance:10,
  maxWanderDistance:100,
  minWanderRadius:5,
  maxWanderRadius:20,
  minWanderStep:0.1,
  maxWanderStep:0.9,
  numBoids:500
};

var drawEnabled = true;
var firstDraw = true;
var boids = [];

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

function initVertexBuffers(gl, first) {
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

function createBoids(){
  for (var i = 0;i < config.numBoids; i++){
    var boid = new Boid();
    boid.color_r = Math.floor(random(100, 255)) / 255;
    boid.color_g = Math.floor(random(100, 255)) / 255;
    boid.color_b = Math.floor(random(100, 255)) / 255;
    boid.edgeBehavior = Boid.EDGE_BOUNCE;
    boid.maxForce = random(config.minForce, config.maxForce);
    boid.maxForceSQ = boid.maxForce*boid.maxForce;
    boid.maxSpeed = random(config.minSpeed, config.maxSpeed);
    boid.maxSpeedSQ = boid.maxSpeed*boid.maxSpeed;
    boid.wanderDistance = random(config.minWanderDistance, config.maxWanderDistance);
    boid.wanderRadius = random(config.minWanderRadius, config.maxWanderRadius);
    boid.wanderStep = random(config.minWanderStep, config.maxWanderStep);
    boid.boundsRadius = STAGE.width/2;
    boid.boundsCentre = new Vector3D(STAGE.width/2, STAGE.height/2, 0.0);
    boid.radius = 16;
    //add positoin and velocity
    boid.position.x = boid.boundsCentre.x + random(-100, 100);
    boid.position.y = boid.boundsCentre.y + random(-100, 100);
    boid.position.z = random(-100, 100);
    var vel = new Vector3D(random(-2, 2), random(-2, 2), random(-2, 2));
    boid.velocity.incrementBy(vel);

    boids.push(boid);
  }
}

function render() {
  for (var i = 0;i < boids.length; i++){
    let boid = boids[i];
    boid.wander(0.3);
    // Add a mild attraction to the centre to keep them on screen
    boid.seek(boid.boundsCentre, 0.1);
    // Flock
    boid.flock(boids);
    boid.update();

    verticesColors[i * 5] = (boid.position.x - canvas.width/2)/(canvas.width/2);
    verticesColors[i * 5 + 1] =
      (canvas.height/2 - boid.position.y)/(canvas.height/2);
    verticesColors[i * 5 + 2] = boid.color_r;
    verticesColors[i * 5 + 3] = boid.color_g;
    verticesColors[i * 5 + 4] = boid.color_b;
  }

  if (drawEnabled){
    // Clear <canvas>
    gl.clear(gl.COLOR_BUFFER_BIT);

    initVertexBuffers(gl, firstDraw);
    
    gl.drawArrays(gl.POINTS, 0, boids.length);
    
    firstDraw = false;
  }
}

//helper classes
function random( min, max ) {
  return Math.random() * ( max - min ) + min;
}
