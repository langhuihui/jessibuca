#pragma once
class NetStream
{
private:
	AudioDecoder* audioDecoder;
	VideoDecoder* videoDecoder;
public:
	MonaClient *mc;
	bool isFirstVideoReceived = true;
	//mp4AudioSpecificConfig* config;
	u8 NAL_unit_length;

	//string sps;
	//string pps;
	bool webgl = false;
	val* jsThis;
	int id;
	//clock_t timestamp;
	time_t  timestamp;
	queue<VideoPacket> videoBuffers;
	bool bufferIsPlaying;
	int bufferTime;
	int videoTimeoutId;
	NetStream(MonaClient* mc) :mc(mc),webgl(false), jsThis(nullptr),videoTimeoutId(0), bufferIsPlaying(0),audioDecoder(nullptr),videoDecoder(nullptr)
	{
		//nc->sendRtmpMessage({ 0x14,0 }, "createStream", nc->createResultFunction({ bind(&NetStream::onConnect,this,placeholders::_1),nullptr }), val::null());
		onConnect(val::null());
	}

	~NetStream()
	{
		emscripten_log(0, "netStream deleted! %d", id);
		EM_ASM_({
			clearTimeout($0)
		}, videoTimeoutId);
		if(videoDecoder)delete videoDecoder;
		if(audioDecoder)delete audioDecoder;
		if (jsThis)	delete jsThis;
	}
	void onConnect(val&& result) {
		id = result.isNull()?0:result.as<int>();
		mc->netStreams[id] = this;
		
		emscripten_log(0, "netStream connect! %d", id);
	}
	void play(string name) {
		mc->Send("[\"__play\",\"" + name + "\"]");
	}
	void attachCanvas(val _this,bool webgl) {
		if(videoDecoder==nullptr){
			this->webgl = webgl;
			emscripten_log(0, "webgl:%s", webgl ? "true" : "false");
			videoDecoder = new VideoDecoder();
			//consoleLog("webgl:%s",webgl?"true":"false");
			if (!jsThis)jsThis = new val(_this);
			videoDecoder->jsObject = jsThis;
			videoDecoder->webgl = webgl;
		}
	}
	
	bool decodeVideo(clock_t _timestamp, MemoryStream& data) {
		if(videoDecoder == nullptr)return false;
		u8 frame_type = data[0];
		int codec_id = frame_type & 0x0f;
		frame_type = (frame_type >> 4) & 0x0f;

		if (codec_id == 7) {
			
		}else if(codec_id == 12){

		}else{
			emscripten_log(0, "Only support video h.264/avc or h.265/hevc codec. actual=%d", data[0]);
			return false;
		}
		u8 avc_packet_type = data[1];
		if (frame_type == 1 && avc_packet_type == 0) {
			if (isFirstVideoReceived) {
				timestamp = getTime() - _timestamp;
				videoDecoder->decodeHeader(data,codec_id);
				isFirstVideoReceived = false;
			}
		}
		else {
			if (!jsThis)return false;
			if(bufferTime)
			{
				auto targetTime = getTime() - timestamp;
				//consoleLog("%d %d", rtmpHeader.timestamp,targetTime);
				if (_timestamp + bufferTime * 1000 > targetTime) {
					videoBuffers.emplace(_timestamp, move(data));
					//emscripten_log(0,"add to video buffer %d", _timestamp);
					//videoBuffers.insert(videoBuffers.begin(), VideoPacket{ _timestamp,data });
					//checkVideoBuffer();
					if(!bufferIsPlaying)
					{
						bufferIsPlaying = true;
						auto&& info = val::object();
						info.set("code", "NetStream.Play.Start");
						jsThis->call<void>("onNetStatus", info);
						emscripten_log(0, "setTimeout to play video buffer %d", _timestamp - targetTime + bufferTime * 1000);
						videoTimeoutId=jsThis->call<int>("checkVideoBuffer", _timestamp - targetTime + bufferTime * 1000);
					}
					return false;
				}
			}
			videoDecoder->decode(data);
		}
		return true;
	}
	void decodeVideoBuffer() {
		if (!videoBuffers.empty()) {
			//emscripten_log(0, "decode video buffer %d", videoBuffers.size());
			videoDecoder->decode(videoBuffers.front().data);
			videoBuffers.pop();
			//emscripten_log(0, "decode video buffer2 %d", videoBuffers.empty());
			if (!videoBuffers.empty()) {
				int time = videoBuffers.front().timestamp;
				auto targetTime = getTime() - timestamp;
				//emscripten_log(0, "decode video buffer time:%d targetTime:%d timestamp:%d", time, targetTime, timestamp);
				if (time + bufferTime * 1000> targetTime) {
					videoTimeoutId=jsThis->call<int>("checkVideoBuffer", time - targetTime + bufferTime * 1000);
					return;
				}
				else {
					decodeVideoBuffer();
				}
			}else
			{
				bufferIsPlaying = false;
			}
		}else
		{
			bufferIsPlaying = false;
			auto&& info = val::object();
			info.set("code","NetStream.Buffer.Empty");
			jsThis->call<void>("onNetStatus", info);
		}
	}

	bool decodeAudio(clock_t _timestamp, MemoryStream & data) {
		if(audioDecoder == nullptr)return false;
		unsigned char flag = 0;
		data.readB<1>(flag);
		auto audioType = flag >> 4;
		if(audioDecoder->decode(audioType, data))
			jsThis->call<void>("playAudio");
		return true;
	}
	int initAudio(val _this,int frameCount, int channels) {
		if(!jsThis)jsThis = new val(_this);
		audioDecoder = new AudioDecoder(frameCount*channels * 2);
		return (int)audioDecoder->outputBuffer>>1;
	}
	val getBufferTime() const
	{
		return val(bufferTime);
	}
	void setBufferTime(val value)
	{
		bufferTime = value.as<int>();
	}
	
	int getTime() const
	{
		return EM_ASM_INT({ return (new Date()).getTime(); },0);
	}
};
EMSCRIPTEN_BINDINGS(NetStream) {
	class_<NetStream>("NetStream")
		.constructor<MonaClient*>(allow_raw_pointers())
		.function("_attachCanvas", &NetStream::attachCanvas)
		.function("_initAudio", &NetStream::initAudio)
		.function("play", &NetStream::play)
		.function("decodeVideoBuffer", &NetStream::decodeVideoBuffer)
		.property("bufferTime", &NetStream::getBufferTime, &NetStream::setBufferTime)
		;
}
