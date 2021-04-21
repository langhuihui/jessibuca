'use strict';

importScripts("../common/helper.js");
importScripts("../common/Vector3D.js");
importScripts("../common/Boid.js");
importScripts("../common/lock.js");

// Basically RPC.
var STAGE = { width:1080, height:1920 };

var lock_compute;
var lock_render;
var cond_compute;
var cond_render;

if (helper.IsWorkerEnv()) {
  self.onmessage = function(msg) {
    console.log('Window post to compute worker:' + msg.data.name);

    if (msg.data.name === "Init") {
      Init(msg.data);
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

var boids = [];
var verticesColors;

function Init(data) {
  STAGE.width = data.width;
  STAGE.height = data.height;

  config.numBoids = data.boids;
  
  let sab = data.buffer;
  let lock_offset = data.boids * 5 * Float32Array.BYTES_PER_ELEMENT;
  lock_compute = new Lock(sab, lock_offset, "Compute");
  cond_compute = new Cond(lock_compute, lock_offset + Lock.NUMBYTES);

  //initialize test variables
  createBoids();

  // Allocate buffer for position and color
  verticesColors = new Float32Array(sab, 0, data.boids * 5);

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

  function computeloop() {
    lock_compute.lock();
    // Wait renderer use computed data
    while (hasComputedData())
      cond_compute.wait();
    // Compute data
    compute();
    addComputedData();
    // Wake renderer to upload data
    cond_compute.wakeOne();
    lock_compute.unlock();
  }

  setInterval(computeloop, 0);
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

function compute() {
  for (var i = 0;i < boids.length; i++){
    let boid = boids[i];
    boid.wander(0.3);
    // Add a mild attraction to the centre to keep them on screen
    boid.seek(boid.boundsCentre, 0.1);
    // Flock
    boid.flock(boids);
    boid.update();

    verticesColors[i * 5] = (boid.position.x - STAGE.width/2)/(STAGE.width/2);
    verticesColors[i * 5 + 1] =
      (STAGE.height/2 - boid.position.y)/(STAGE.height/2);
    verticesColors[i * 5 + 2] = boid.color_r;
    verticesColors[i * 5 + 3] = boid.color_g;
    verticesColors[i * 5 + 4] = boid.color_b;
  }
}

//helper classes
function random( min, max ) {
  return Math.random() * ( max - min ) + min;
}
