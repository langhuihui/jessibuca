#pragma once
#include "handshake.h"
#include <emscripten\emscripten.h>
#include "NetStream.h"
#include <math.h>
inline int getRealChunkSize(int rtmpBodySize, int chunkSize) {
	int nn = rtmpBodySize + (rtmpBodySize / chunkSize);
	if (rtmpBodySize % chunkSize) {
		return nn;
	}
	else {
		return nn - 1;
	}
}
class NetConnection {
private:
	bool handshaked = false;
	int outChunkSize = 128;
	int inChunkSize = 128;
	MemoryStream buffer;
	map<byte, RtmpHeader> previousChunkMessage;
	vector<Response> resultFunctions;
public:
	val*  ws = nullptr;
	val* client = nullptr;
	val* netStatusLisenter = nullptr;
	val* jsThis = nullptr;
	string url;
	string appName;
	string roomName;
	
	map<int, NetStream*> netStreams;
	NetConnection() :previousChunkMessage(), resultFunctions()
		
	{
	}
	~NetConnection() {
		if (netStatusLisenter)delete netStatusLisenter;
		if (client)delete client;
		if (jsThis)delete jsThis;
		if (ws)delete ws;
	}
	int createResultFunction(Response&& response) {
		int len = resultFunctions.size();
		resultFunctions.push_back(move(response));
		return len;
	}

	val connect(val _this,string url,string appName,string roomName) {
		if (!jsThis)jsThis = new val(_this);
		val WebSocket = val::global("WebSocket");
		if (ws)free(ws);
		ws = new val(WebSocket.new_("ws://" + url));
		//ws->set("binaryType", "arraybuffer");
		ws->set("onopen", _this["onWsOpen"].call<val>("bind",_this));
		ws->set("onmessage", _this["onWsMessage"].call<val>("bind", _this));
		this->appName = appName;
		this->roomName = roomName;
		return *ws;
	}
	void onWsOpen(val evt) {
		emscripten_log(0,"websocket open!");
		string buffer(1537, '\0');
		buffer[0] = 3;
		for (int i = 9; i < 1537; i++) {
			buffer[i] = rand() % 256;
		}
		buffer[5] = 9;
		buffer[6] = 0;
		buffer[7] = 124;
		buffer[8] = 2;
		int sdl = GetClientGenuineConstDigestOffset(buffer, 9)+1;
		string temp = buffer.substr(1, sdl-1) + buffer.substr(sdl + 32);
		temp = rstr_hmac_sha256("Genuine Adobe Flash Player 001", temp);
		for (int i = 0; i < 32; i++) {
			buffer[sdl+ i] = temp[i];
		}
		send(buffer);
	}
	void onWsMessage(val evt) {
		string data = evt["data"].as<string>();
		if (!handshaked) {
			data = data.substr(1);
			int sdl = GetClientGenuineConstDigestOffset(data, 8);
			string buffer(1504, '\0');
			for (int i = 0; i < 1504; i++) {
				buffer[i] = (rand() % 256);
			}
			string hash = rstr_hmac_sha256(rstr_hmac_sha256((char*)GenuineFPKey, data.substr(sdl, 32)), buffer);
			buffer.append(hash);
			val cmdObj = val::object();
			cmdObj.set("app", appName);
			string tcUrl = this->url + "/" + this->appName;
			if (this->roomName.length()) {
				tcUrl.append("/");
				tcUrl.append(this->roomName);
			}
			cmdObj.set("tcUrl", tcUrl);
			cmdObj.set("flashVer", "WIN 17,0,0,134");
			cmdObj.set("fpad", false);
			cmdObj.set("capabilities", 235.0);
			cmdObj.set("audioCodecs", 3575.0);
			cmdObj.set("videoCodecs", 252.0);
			cmdObj.set("videoFunction",1.0);
			cmdObj.set("swfUrl", "");
			cmdObj.set("pageUrl", "");
			cmdObj.set("objectEncoding", 0);
			hash = CreateRtmpMessage({3,0,0x14,0,0,0,0}, encodeAmf0Cmd("connect", 0, cmdObj, val::null()));
			buffer.append(hash);
			send(buffer);
			handshaked = true;
			emscripten_log(0, "handshake!");
		}
		else {
			buffer << data;
			parseRtmpMessage();
		}
	}
	static val Test(val data,val arg) {
		return data.call<val>("apply",val::null(),arg);
	}

	val getWebSocket() {
		return *ws;
	}
	inline void send(const string& buffer) {
		//int len = buffer.length();
		//int ptr = (int)buffer.c_str();
		ws->call<void>("send",buffer);
		//jsThis->call<void>("send",ptr,len,buffer);
	}
	val getClient() const { return client==nullptr?val::null():*client; }
	void setClient(val _client) { client = new val(_client);}
	val getOnNetStatus() const { return netStatusLisenter == nullptr ? val::null() : *netStatusLisenter; }
	void setOnNetStatus(val _netStatusLisenter) { netStatusLisenter =new val(_netStatusLisenter); }
	void handleAMFCommandMessage(val cmd) {
		string command = cmd["cmd"].as<string>();
		int transId = cmd["transId"].as<int>();
		emscripten_log(0, "%s",command.c_str());
		if (command == "_result") {
			if (cmd["params"][0].hasOwnProperty("code")) {
				if(netStatusLisenter != nullptr)netStatusLisenter->call<void>("apply", val::null(), cmd["params"]);
			}
			else {
				try {
					resultFunctions[transId].result(cmd["params"][0]);
				}
				catch(exception exp){
					emscripten_log(0, "%s", exp.what());
				}
			}
		}
		else if (command == "_error") {
			if (cmd["params"][0].hasOwnProperty("code")) {
				emscripten_log(0, "%s", cmd["params"][0]["code"].as<string>().c_str());
				if (netStatusLisenter != nullptr)netStatusLisenter->call<void>("apply", val::null(), cmd["params"]);
			}
			else {
				try {
					resultFunctions[transId].error(cmd["params"][0]);
				}
				catch (exception exp) {

				}
			}
		}
		else if (command == "onStatus") {
			if (netStatusLisenter != nullptr)netStatusLisenter->call<void>("apply", val::null(), cmd["params"]);
		}
		else {
			if (client != nullptr) {
				(*client)[command].call<void>("apply",val::null(), cmd["params"]);
			}
		}
	}
	void handleRtmpMessage(RtmpHeader& rtmpHeader,MemoryStream& rtmpBody) {
		//emscripten_log(0,"handleRtmpMessage %d,%d,%d", rtmpHeader.messageTypeID,rtmpHeader.messageStreamID, rtmpHeader.timestamp);
		switch (rtmpHeader.messageTypeID) {
		case AMF::CHUNKSIZE:
			rtmpBody.read4B(inChunkSize);
			break;
		case AMF::RAW:
			break;
		case AMF::AUDIO:
			netStreams[rtmpHeader.messageStreamID]->decodeAudio(rtmpHeader,rtmpBody);
			break;
		case AMF::VIDEO:
			netStreams[rtmpHeader.messageStreamID]->decodeVideo(rtmpHeader,rtmpBody);
			break;
		case AMF::INVOCATION:
			handleAMFCommandMessage(decodeAmf0Cmd(rtmpBody));
			break;
		}
	}
	
	void parseRtmpMessage() {
		RtmpHeader rtmpHeader;
		string chunkMessageHeader;
		byte chunkBasicHeader = buffer.readByte();
		rtmpHeader.formatType = chunkBasicHeader >> 6;
		rtmpHeader.chunkStreamID = chunkBasicHeader & 0x3F;
		if (rtmpHeader.chunkStreamID == 0) {
			rtmpHeader.chunkStreamID = buffer.readByte() + 64;
		}
		else if (rtmpHeader.chunkStreamID == 1) {
			rtmpHeader.chunkStreamID = buffer.readUInt16B() + 64;
		}
		switch (rtmpHeader.formatType) {
			case 0:
				rtmpHeader.timestampDelta = 0;
				buffer.read3B(rtmpHeader.timestamp)
					.read3B(rtmpHeader.messageLength)
					.read1(rtmpHeader.messageTypeID)
					.readNormal(rtmpHeader.messageStreamID);
				break;
			case 1:
			{
				RtmpHeader& previousChunk = previousChunkMessage[rtmpHeader.chunkStreamID];
				buffer.read3B(rtmpHeader.timestampDelta)
					.read3B(rtmpHeader.messageLength)
					.read1(rtmpHeader.messageTypeID);
				rtmpHeader.timestamp = previousChunk.timestamp;
				rtmpHeader.messageStreamID = previousChunk.messageStreamID;
			}
				break;
			case 2:
			{
				RtmpHeader& previousChunk = previousChunkMessage[rtmpHeader.chunkStreamID];
				buffer.read3B(rtmpHeader.timestampDelta);
				rtmpHeader.messageLength = previousChunk.messageLength;
				rtmpHeader.messageTypeID = previousChunk.messageTypeID;
				rtmpHeader.timestamp = previousChunk.timestamp;
				rtmpHeader.messageStreamID = previousChunk.messageStreamID;
			}
				break;
			case 3:
			{
				RtmpHeader& previousChunk = previousChunkMessage[rtmpHeader.chunkStreamID];
				rtmpHeader.timestampDelta = previousChunk.timestampDelta;
				rtmpHeader.messageLength = previousChunk.messageLength;
				rtmpHeader.messageTypeID = previousChunk.messageTypeID;
				rtmpHeader.timestamp = previousChunk.timestamp;
				rtmpHeader.messageStreamID = previousChunk.messageStreamID;
			}
		}
		int rtmpBodySize = rtmpHeader.messageLength;
		int chunkBodySize = getRealChunkSize(rtmpBodySize, inChunkSize);
		if (buffer.length() < chunkBodySize) {
			buffer.offset = 0;
			return;
		}
		//Extended Timestamp
		if (rtmpHeader.formatType == 0) {
			if (rtmpHeader.timestamp == 0xffffff) {
				buffer.read4B(rtmpHeader.timestamp);
			}
		}
		else if (rtmpHeader.timestampDelta == 0xffffff) {
			buffer.read4B(rtmpHeader.timestampDelta);
		}
		MemoryStream rtmpBodyBuf;
		rtmpHeader.timestamp += rtmpHeader.timestampDelta;
		previousChunkMessage[rtmpHeader.chunkStreamID] = rtmpHeader;
		do {
			if (rtmpBodySize > inChunkSize) {
				rtmpBodyBuf << buffer.readString(inChunkSize);
				rtmpBodySize -= inChunkSize;
				buffer >>= 1;//head
			}
			else {
				rtmpBodyBuf << buffer.readString(rtmpBodySize);
				rtmpBodySize -= rtmpBodySize;
			}
		} while (rtmpBodySize > 0);
		handleRtmpMessage(rtmpHeader, rtmpBodyBuf);
		buffer.clear();
	}
	string CreateRtmpMessage(const RtmpHeader& rtmpHeader, string data) {
		int rtmpBodySize = data.length();
		bool useExtendedTimestamp = false;
		byte _timestamp0 = 0xFF;
		byte _timestamp1 = 0xFF;
		byte _timestamp2 = 0xFF;
		if (rtmpHeader.timestamp >= 0xffffff) {
			useExtendedTimestamp = true;
		}
		else {
			_timestamp0 = (rtmpHeader.timestamp >> 16) & 0xff;
			_timestamp1 = (rtmpHeader.timestamp >> 8) & 0xff;
			_timestamp2 = (rtmpHeader.timestamp) & 0xff;
		}
		string bufs(12, '\0');
		bufs[0] = (rtmpHeader.formatType << 6) | rtmpHeader.chunkStreamID;
		bufs[1] = _timestamp0;
		bufs[2] = _timestamp1;
		bufs[3] = _timestamp2;
		bufs[4] = (rtmpBodySize >> 16) & 0xff;
		bufs[5] = (rtmpBodySize >> 8) & 0xff;
		bufs[6] = rtmpBodySize & 0xff;
		bufs[7] = rtmpHeader.messageTypeID;
		bufs[8] = rtmpHeader.messageStreamID & 0xff;
		bufs[9] = (rtmpHeader.messageStreamID >> 8) & 0xff;
		bufs[10] = (rtmpHeader.messageStreamID >> 16) & 0xff;
		bufs[11] = (rtmpHeader.messageStreamID >> 24) & 0xff;
		if (useExtendedTimestamp) {
			bufs += (rtmpHeader.timestamp >> 24) & 0xff;
			bufs += (rtmpHeader.timestamp >> 16) & 0xff;
			bufs += (rtmpHeader.timestamp >> 8) & 0xff;
			bufs += (rtmpHeader.timestamp) & 0xff;
		}
		int rtmpBodyPos = 0;
		//int chunkBodySize = this->getRealChunkSize(rtmpBodySize, this->outChunkSize);
		char type3Header = (3 << 6) | rtmpHeader.chunkStreamID;
		do {
			if (rtmpBodySize > this->outChunkSize) {
				bufs.append(data.substr(rtmpBodyPos, this->outChunkSize));
				rtmpBodySize -= this->outChunkSize;
				rtmpBodyPos += this->outChunkSize;
				bufs += type3Header;
			}
			else {
				bufs.append(data.substr(rtmpBodyPos, this->outChunkSize));
				rtmpBodySize -= rtmpBodySize;
				rtmpBodyPos += rtmpBodySize;
			}

		} while (rtmpBodySize > 0);
		return bufs;
	}
	void close() {
		ws->call<void>("close");
	}
	inline void sendRtmpMessage(const RtmpHeader& rtmpHeader,const char* funcName,int transId,val params) {
		send(CreateRtmpMessage(rtmpHeader, encodeAmf0Cmd(funcName,transId,val::null(),params)));
	}
	void call(string funcName,val resp,val params) {
		int transId = 0;
		if (resp.isNull()) {

		}
		else {
			auto result = [&resp](val arg) {
				resp["result"].call<void>("apply", val::null(), arg);
			};
			if (resp.hasOwnProperty("error")) {
				transId = createResultFunction({ result,[&resp](val arg) {
					resp["error"].call<void>("apply", val::null(), arg);
				} });
			}
			else {
				transId = createResultFunction({ result,nullptr });
			}
		}
		sendRtmpMessage({3,0,0x14,0,0,0,0}, funcName.c_str(), transId, params);
	}
};

EMSCRIPTEN_BINDINGS(NetConnection) {
	class_<NetConnection>("NetConnection")
		.constructor()
		.function("onWsMessage", &NetConnection::onWsMessage)
		.function("onWsOpen", &NetConnection::onWsOpen)
		.function("getWebSocket",&NetConnection::getWebSocket)
		.function("close", &NetConnection::close)
		.function("$call", &NetConnection::call)
		.function("$connect", &NetConnection::connect)
		.property("client",&NetConnection::getClient, &NetConnection::setClient)
		.property("onNetStatus", &NetConnection::getOnNetStatus, &NetConnection::setOnNetStatus);
		//.field("ws",&NetConnection::ws)
		;
}

NetStream::NetStream(NetConnection* _nc) :nc(_nc),
webgl(false),jsThis(nullptr),heap(0), timeoutId(0), canvasAttached(false)
{
	nc->sendRtmpMessage({ 3,0,0x14,0,0,0,0 }, "createStream", nc->createResultFunction({ bind(&NetStream::onConnect,this,placeholders::_1),nullptr }), val::null());
}
void NetStream::onConnect(val&& result) {
	id = result.as<int>();
	emscripten_log(0, "netStream connect! %d",id);
	nc->netStreams[id] = this;
	H264SwDecRet ret;
	u32 disableOutputReordering = 0;

	/* Initialize decoder instance. */
	ret = H264SwDecInit(&decInst, disableOutputReordering);
	if (ret != H264SWDEC_OK) {
		emscripten_log(0, "DECODER INITIALIZATION FAILED\n");
		return;
	}
	picDecodeNumber = picDisplayNumber = 1;
}
void NetStream::play(string name) {
	val params = val::array(); 
	params.set(0, name);
	nc->sendRtmpMessage({ 3,0,0x14,(unsigned) this->id,0,0,0 },"play",0, params);
}
EMSCRIPTEN_BINDINGS(NetStream) {
	class_<NetStream>("NetStream")
		.constructor<NetConnection*>(allow_raw_pointers())
		.function("_attachCanvas", &NetStream::attachCanvas)
		.function("_initAudio", &NetStream::initAudio)
		.function("play", &NetStream::play)
		.function("decodeVideoBuffer",&NetStream::decodeVideoBuffer)
		;
}
