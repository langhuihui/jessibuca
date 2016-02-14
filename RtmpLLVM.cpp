#include "NetConnection.h"
int main() {
	EM_ASM(
		var nc = window["NetConnection"] = Module["NetConnection"];
		var ns = window["NetStream"] = Module["NetStream"];
		ns.prototype.attachCanvas = function(canvas) {
			var webGLCanvas = new WebGLCanvas(canvas, Module["noWebGL"], {});
			this.setVideoSize = function( w, h, dataPtr) {
				canvas.width = w;
				canvas.height = h;
				if (webGLCanvas.isWebGL()) {
					this.draw = function(ptr) {
						var outputArray = HEAPU8.subarray(ptr, ptr + (w * h * 3) / 2);
						webGLCanvas.drawNextOutputPicture(w, h, null, outputArray);
					};
				}
				else {
					var outputArray = HEAPU8.subarray(dataPtr, dataPtr + (w*h << 2));
					webGLCanvas.initRGB(w, h);
					this.draw = function() {
						webGLCanvas.drawNextOutputPicture(w, h, null, outputArray);
					};
				}
			}.bind(this);
			this._attachCanvas(this, webGLCanvas.isWebGL());
		};
		ns.prototype.initAudio = function(bufferFrames) {
			var isPlaying = false;
			var audioBuffers = [];
			var len = 320 * bufferFrames;
			var audioBufferTime = bufferFrames*20;	
			var AudioContext = AudioContext || webkitAudioContext || mozAudioContext || msAudioContext;
			var audioCtx = new AudioContext();
			if (!audioCtx)return false;
			//alert(audioCtx.createBuffer(1, len, 32000));
			var audioBuffer = audioCtx.createBuffer(1, len<<1, 32000);
			
			var outputPtr = this._initAudio(this, bufferFrames);
			var audioOutputArray = HEAP16.subarray(outputPtr>>1,(outputPtr>>1)+ 320 * bufferFrames);
			var setCurrentAudioData = function(target) {
				for (var i = 0; i < len; i++) {
					var j = i << 1;
					target[j] = target[j+1] = audioOutputArray[i] / 32768;
				}
			};
			var playNextBuffer = function() {
				isPlaying = false;
				if (audioBuffers.length) {
					playAudio(audioBuffers.shift());
				}
			};
			var playAudio = function(fromBuffer) {
				if (isPlaying) {
					var buffer = new Float32Array(len<<1);
					setCurrentAudioData(buffer);
					audioBuffers.push(buffer);
					return;
				}
				isPlaying = true;
				var nowBuffering = audioBuffer.getChannelData(0);
				if (fromBuffer) nowBuffering.set(fromBuffer);
				else setCurrentAudioData(nowBuffering);
				var source = audioCtx.createBufferSource();
				source.buffer = audioBuffer;
				source.connect(audioCtx.destination);
				setTimeout(playNextBuffer, audioBufferTime);
				source.start();
			};
			this.playAudio = playAudio;
		};
		ns.prototype.checkVideoBuffer = function(t) {
			return setTimeout(this.decodeVideoBuffer.bind(this),t);
		};
		nc.prototype.connect = function(url, appName, roomName) {
			var ws = this.$connect(this,url,appName,roomName||"");
			this.send = function(ptr, length, output) {
				var outputArray = HEAPU8.subarray(ptr, ptr + length);
				ws.send(output);
			};
		};
		nc.prototype.call = function() {
			var args = [].concat(arguments);
			var name = args.shift();
			var response =args.length? args.shift():null;
			this.$call(name, response, args);
		};
	);
	return 0;
}
/*------------------------------------------------------------------------------

Function name:  H264SwDecMalloc

Purpose:
Example implementation of H264SwDecMalloc function. Prototype of this
function is given in H264SwDecApi.h. This implementation uses
library function malloc for allocation of memory.

------------------------------------------------------------------------------*/
void* H264SwDecMalloc(u32 size)
{

#if defined(CHECK_MEMORY_USAGE)
	/* Note that if the decoder has to free and reallocate some of the buffers
	* the total value will be invalid */
	static u32 numBytes = 0;
	numBytes += size;
	DEBUG(("Allocated %d bytes, total %d\n", size, numBytes));
#endif

	return malloc(size);
}

/*------------------------------------------------------------------------------

Function name:  H264SwDecFree

Purpose:
Example implementation of H264SwDecFree function. Prototype of this
function is given in H264SwDecApi.h. This implementation uses
library function free for freeing of memory.

------------------------------------------------------------------------------*/
void H264SwDecFree(void *ptr)
{
	free(ptr);
}

/*------------------------------------------------------------------------------

Function name:  H264SwDecMemcpy

Purpose:
Example implementation of H264SwDecMemcpy function. Prototype of this
function is given in H264SwDecApi.h. This implementation uses
library function memcpy to copy src to dest.

------------------------------------------------------------------------------*/
void H264SwDecMemcpy(void *dest, void *src, u32 count)
{
	memcpy(dest, src, count);
}

/*------------------------------------------------------------------------------

Function name:  H264SwDecMemset

Purpose:
Example implementation of H264SwDecMemset function. Prototype of this
function is given in H264SwDecApi.h. This implementation uses
library function memset to set content of memory area pointed by ptr.

------------------------------------------------------------------------------*/
void H264SwDecMemset(void *ptr, i32 value, u32 count)
{
	memset(ptr, value, count);
}