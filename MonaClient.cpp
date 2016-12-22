#include "MonaClient.h"
#include "NetStream.h"
int main() {
	EM_ASM(
		var mc = window["MonaClient"] = Module["MonaClient"];
		var ns = window["NetStream"] = Module["NetStream"];
		ns.prototype.attachCanvas = function(canvas) {
			var webGLCanvas = new WebGLCanvas(canvas, Module["noWebGL"], {});
			this.setVideoSize = function(w, h, dataPtr)
			{
				canvas.width = w;
				canvas.height = h;
				if (webGLCanvas.isWebGL())
				{
					this.draw = function() {
						var y = HEAPU32[dataPtr];
						var u = HEAPU32[dataPtr + 1];
						var v = HEAPU32[dataPtr + 2];
						var outputArray = [HEAPU8.subarray(y, y + w*h), HEAPU8.subarray(u, u + (w*h >> 2)), HEAPU8.subarray(v, v + (w*h >> 2))];
						webGLCanvas.drawNextOutputPicture(w, h, null, outputArray);
					};
				}else
				{
					var outputArray = HEAPU8.subarray(dataPtr, dataPtr + (w*h << 2));
					webGLCanvas.initRGB(w, h);
					this.draw = function() {
						webGLCanvas.drawNextOutputPicture(w, h, null, outputArray);
					};
				}
			}.bind(this);
			
			this.getWebGL = function()
			{
				return webGLCanvas;
			};
			this._attachCanvas(this, webGLCanvas.isWebGL());
		};
		ns.prototype.checkVideoBuffer = function(t) {
			return setTimeout(this.decodeVideoBuffer.bind(this),t);
		};
		ns.prototype.onNetStatus = function(info)
		{
			
		};
		ns.prototype.initAudio = function(frameCount,samplerate,channels) {
			var isPlaying = false;
			var audioBuffers = [];
			var allFrameCount = frameCount*channels;
			var audioBufferTime = frameCount * 1000 /samplerate;
			var AudioContext = window.AudioContext || window.webkitAudioContext || window.mozAudioContext || window.msAudioContext;
			var audioCtx = new AudioContext();
			if (!audioCtx)return false;
			var setCurrentAudioData;
			var resampled = samplerate < 22050;
			var audioBuffer = resampled? audioCtx.createBuffer(channels, frameCount<<1, samplerate<<1) : audioCtx.createBuffer(channels, frameCount, samplerate);
			var outputPtr = this._initAudio(this, frameCount, channels);
			var audioOutputArray = HEAP16.subarray(outputPtr, outputPtr + allFrameCount);
			var playNextBuffer = function() {
				isPlaying = false;
				if (audioBuffers.length) {
					playAudio(audioBuffers.shift());
				}
			};
			var copyAudioOutputArray = resampled ? function(target)
			{
				for (var i = 0; i < allFrameCount; i++) {
					var j = i << 1;
					target[j]  = target[j+1] = audioOutputArray[i] / 32768;
				}
			} : function(target)
			{
				for (var i = 0; i < allFrameCount; i++) {
					
					target[i] = audioOutputArray[i] / 32768;
				}
			};
			var copyToCtxBuffer =channels>1? function(fromBuffer)
			{
				for (var channel = 0; channel < channels; channel++)
				{
					var nowBuffering = audioBuffer.getChannelData(channel);
					if (fromBuffer)
					{
						for (var i = 0; i < frameCount; i++) {
							nowBuffering[i] = fromBuffer[i* (channel + 1)];
						}
					}
					else
					{
						for (var i = 0; i < frameCount; i++) {
							nowBuffering[i] = audioOutputArray[i* (channel + 1)] / 32768;
						}
					}
				}
			}:function(fromBuffer)
			{
				var nowBuffering = audioBuffer.getChannelData(0);
				if (fromBuffer)nowBuffering.set(fromBuffer);
				else copyAudioOutputArray(nowBuffering);
			};
			var playAudio = function(fromBuffer) {
				if (isPlaying) {
					var buffer = new Float32Array(resampled ? allFrameCount *2: allFrameCount);
					copyAudioOutputArray(buffer);
					audioBuffers.push(buffer);
					return;
				}
				isPlaying = true;
				copyToCtxBuffer(fromBuffer);
				var source = audioCtx.createBufferSource();
				source.buffer = audioBuffer;
				source.connect(audioCtx.destination);
				setTimeout(playNextBuffer, audioBufferTime);
				source.start();
			};
			this.playAudio = playAudio;
		};
		mc.prototype.connect = function(url, appName, roomName) {
			var ws = this.$connect(this, url, appName, roomName || "");
			this.send = function(ptr, length, output) {
				var outputArray = HEAPU8.subarray(ptr, ptr + length);
				ws.send(outputArray);
			};
		};
	);
	return 0;
}
MonaClient::MonaClient()
{
	/*char packet_bytes[] = {
		0x00, 0x00, 0x00, 0x00, 0x01, 0x42, 0x00, 0x1f,
		0x03, 0x01, 0x00, 0x2f, 0x67, 0x42, 0x80, 0x1f,
		0x96, 0x52, 0x02, 0x83, 0xf6, 0x02, 0xa1, 0x00,
		0x00, 0x03, 0x00, 0x01, 0x00, 0x00, 0x03, 0x00,
		0x3c, 0xe0, 0x60, 0x03, 0x0d, 0x40, 0x00, 0x46,
		0x30, 0xff, 0x18, 0xe3, 0x03, 0x00, 0x18, 0x6a,
		0x00, 0x02, 0x31, 0x87, 0xf8, 0xc7, 0x0e, 0xd0,
		0xa1, 0x52, 0x40, 0x01, 0x00, 0x04, 0x68, 0xcb,
		0x8d, 0x48
	};*/
	//u8 packet_bytes[] = {
	//	0x02, 0x00, 0x00, 0x00, 0x16, 0x1c, 0x00, 0x00,
	//	0x00, 0x00, 0x01, 0x01, 0x60, 0x00, 0x00, 0x00,
	//	0xb0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x5d, 0xf0,
	//	0x00, 0xfc, 0xfd, 0xf8, 0xf8, 0x00, 0x00, 0x0f,
	//	0x03, 0x20, 0x00, 0x01, 0x00, 0x18, 0x40, 0x01,
	//	0x0c, 0x01, 0xff, 0xff, 0x01, 0x60, 0x00, 0x00,
	//	0x03, 0x00, 0xb0, 0x00, 0x00, 0x03, 0x00, 0x00,
	//	0x03, 0x00, 0x5d, 0xaa, 0x02, 0x40, 0x21, 0x00,
	//	0x01, 0x00, 0x1d, 0x42, 0x01, 0x01, 0x01, 0x60,
	//	0x00, 0x00, 0x03, 0x00, 0xb0, 0x00, 0x00, 0x03,
	//	0x00, 0x00, 0x03, 0x00, 0x5d, 0xa0, 0x02, 0x00,
	//	0x80, 0x30, 0x16, 0x36, 0xaa, 0x49, 0x32, 0xf9,
	//	0x22, 0x00, 0x01, 0x00, 0x07, 0x44, 0x01, 0xc0,
	//	0xf2, 0xf0, 0x3c, 0x90
	//};
	//MemoryStream ms;
	//ms << (char*)packet_bytes;
	//ms.offset = 38;
	//VideoDecoder decoder;
	//decoder.decode(ms.point(), ms.length());
}
MonaClient::~MonaClient()
{
	delete ws;
	delete client;
	delete netStatusLisenter;
	delete jsThis;
}

void MonaClient::OnWsMessage(val evt)
{
	string&& dataType = evt["data"].typeof().as<string>();
	string data = evt["data"].as<string>();
	switch (data.at(0)) {
		case 1:
		{
			MemoryStream ms(data.substr(1));
			netStreams[0]->decodeAudio(ms.readUInt32B(), ms);
		}
			break;
		case 2: 
		{	
			MemoryStream ms(data.substr(1));
			netStreams[0]->decodeVideo(ms.readUInt32B(), ms);
		}
			break;
		default:
		{
			//val&& result = val::global("window").call<val>("eval", data);
			//string& command = result[0].as<string>();
			//emscripten_log(0, command.c_str());
			emscripten_log(0, data.c_str());
			jsThis->call<void>("onWsMessage",data);
		}
			break;
	}
	
}
void MonaClient::OnWsOpen(val evt)
{
	emscripten_log(0, "websocket open!");
	jsThis->call<void>("onWsOpen");
}
val MonaClient::GetWebSocket(val evt)
{
	return *ws;
}
void MonaClient::Send(const string& buffer) const
{
#ifdef USEBUFFERARRAY
	int len = buffer.length();
	int ptr = (int)buffer.c_str();
	jsThis->call<void>("send", ptr, len, buffer);
#else
	ws->call<void>("send", buffer);
#endif
}
val MonaClient::Connect(val _this, string url, string appName, string roomName)
{
	
	if (!jsThis)jsThis = new val(_this);
	val WebSocket = val::global("WebSocket");
	if (ws) {
		ws->call<void>("close");
		delete ws;
	}
	ws = new val(WebSocket.new_("ws://" + url+"/"+appName+"/"+roomName));
	ws->set("binaryType", "arraybuffer");
	ws->set("onopen", _this["$onWsOpen"].call<val>("bind", _this));
	ws->set("onmessage", _this["$onWsMessage"].call<val>("bind", _this));
	ws->set("onerror", _this["onWsError"]);
	ws->set("onclose", _this["onWsClose"]);
	this->appName = appName;
	this->roomName = roomName;
	return *ws;
}
void MonaClient::Close()
{
	ws->call<void>("close");
}
EMSCRIPTEN_BINDINGS(MonaClient) {
	class_<MonaClient>("MonaClient")
		.constructor()
		.function("$onWsMessage", &MonaClient::OnWsMessage)
		.function("$onWsOpen", &MonaClient::OnWsOpen)
		.function("getWebSocket", &MonaClient::GetWebSocket)
		.function("close", &MonaClient::Close)
		//.function("$call", &MonaClient::call)
		.function("$connect", &MonaClient::Connect)
		//.property("client", &MonaClient::getClient, &MonaClient::setClient)
		//.property("onNetStatus", &MonaClient::getOnNetStatus, &MonaClient::setOnNetStatus);
	//.field("ws",&NetConnection::ws)
	;
}