#pragma once
#include "base.h"
class NetStream;
class MonaClient
{
	val* ws = nullptr;
	val* client = nullptr;
	val* netStatusLisenter = nullptr;
	val* jsThis = nullptr;
	string url;
	string appName;
	string roomName;
public:
	map<int, NetStream*> netStreams;
	MonaClient();
	~MonaClient();
	void OnWsMessage(val evt);
	void OnWsOpen(val evt);
	val GetWebSocket(val evt);
	val Connect(val _this, string url, string appName, string roomName);
	void Close();
	void Send(const string& buffer) const;
};