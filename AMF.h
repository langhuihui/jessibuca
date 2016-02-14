/*

Copyright 2014 Mona
mathieu.poux[a]gmail.com
jammetthomas[a]gmail.com

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License received along this program for more
details (or else see http://www.gnu.org/licenses/).

This file is a part of Mona.
*/

#pragma once

#define AMF_NULL			0x05
#define AMF_UNDEFINED		0x06
#define AMF_UNSUPPORTED		0x0D
#define	AMF_AVMPLUS_OBJECT	0x11

#define AMF_NUMBER			0x00
#define AMF_BOOLEAN			0x01
#define AMF_STRING			0x02
#define AMF_DATE			0x0B

#define AMF_BEGIN_OBJECT		0x03
#define AMF_BEGIN_TYPED_OBJECT	0x10
#define AMF_END_OBJECT			0x09
#define AMF_REFERENCE			0x07

#define AMF_MIXED_ARRAY	    0x08 
#define	AMF_STRICT_ARRAY	0x0A

#define	AMF_LONG_STRING		0x0C


#define AMF3_UNDEFINED		0x00
#define AMF3_NULL			0x01
#define AMF3_FALSE			0x02
#define AMF3_TRUE			0x03
#define AMF3_INTEGER		0x04
#define AMF3_NUMBER			0x05
#define AMF3_STRING			0x06
#define AMF3_DATE			0x08
#define AMF3_ARRAY			0x09
#define AMF3_OBJECT			0x0A
#define AMF3_BYTEARRAY		0x0C
#define AMF3_DICTIONARY		0x11

#define	AMF_END				0xFF

#define AMF_MAX_INTEGER		268435455


class AMF {
public:
	enum ContentType {
		EMPTY				=0x00,
		CHUNKSIZE			=0x01,
		ABORT				=0x02,
		ACK					=0x03,
		RAW					=0x04,
		WIN_ACKSIZE			=0x05,
		BANDWITH			=0x06,
		AUDIO				=0x08,
		VIDEO				=0x09,
		DATA_AMF3			=0x0F,
		INVOCATION_AMF3		=0x11,
		DATA				=0x12,
		INVOCATION			=0x14
	};
};
struct RtmpHeader {
	unsigned int chunkStreamID;
	unsigned int timestamp;
	byte messageTypeID;
	unsigned int messageStreamID;

	byte formatType;
	unsigned int messageLength;
	unsigned int timestampDelta;
};
struct RtmpPacket {
	RtmpHeader rtmpHeader;
	MemoryStream data;
	RtmpPacket(RtmpHeader& rtmpHeader, MemoryStream&ms) :rtmpHeader(rtmpHeader), data(ms) {

	}
	RtmpPacket(const RtmpPacket& copy) :rtmpHeader(copy.rtmpHeader), data(copy.data) {

	}
	RtmpPacket& operator = (const RtmpPacket & copy) {
		rtmpHeader = copy.rtmpHeader;
		data = copy.data;
		return *this;
	}
};
template <class T, class S> inline void writeBE(int size, S source,T& target, int offset) {
	for (int i = 0; i < size; i++) {
		target[offset+i] = ((char*)(&source))[size - i - 1];
	}
}

int amf0Type(val o) {
	string jsType = o.typeof().as<string>();
	if(o.isNull()) return AMF_NULL;
	if (o.isUndefined())return AMF_UNDEFINED;
	if (jsType == "number") return AMF_NUMBER;
	if (jsType == "boolean") return AMF_BOOLEAN;
	if (jsType == "string") return AMF_STRING;
	if (jsType == "object") {
		if (o["__proto__"].equals(val::global("Array")["prototype"])) {
			return AMF_MIXED_ARRAY;
		}
		return AMF_BEGIN_OBJECT;
	}
	return NULL;
}
string amf0EncodeOne(val data);
string amf0encObject(val data) {
	string result = "";
	val names = val::global("Object").call<val>("getOwnPropertyNames",data);
	int len = names["length"].as<int>();
	for (int i = 0; i < len; i++) {
		string name = names[i].as<string>();
		int offset = result.length();
		result.resize(offset + 2);
		writeBE(2, (unsigned short)name.length(), result, offset);
		result.append(name);
		val o = data[name];
		result.append(amf0EncodeOne(o));
	}
	result += (char)0;
	result += (char)0;
	result += (char)AMF_END_OBJECT;
	return result;
}
string amf0EncodeOne(val data) {
	int type = amf0Type(data);
	string result((size_t)1,(char)type);
	switch (type) {
	case AMF_NULL:
	case AMF_UNDEFINED:
		break;
	case AMF_NUMBER: 
	{
		result.resize(9);
		writeBE(8, data.as <double>(), result, 1);
	}
		break;
	case AMF_BOOLEAN:
		result += (char)(data.isTrue() ? 1:0);
		break;
	case AMF_STRING:
		result.resize(3);
		{
			string str = data.as<string>();
			writeBE(2, (unsigned short)str.length(), result, 1);
			result.append(str);
		}
		break;
	case AMF_MIXED_ARRAY:
		result.resize(5);
		{
		writeBE(4, data["length"].as<int>(), result, 1);
		result += amf0encObject(data);
		}
		break;
	case AMF_BEGIN_OBJECT:
		result.append(amf0encObject(data));
		break;
	}
	return result;
}

string encodeAmf0Cmd(const char* cmd,int transId,val cmdObj,val params) {
	string data = amf0EncodeOne(val(cmd)) + amf0EncodeOne(val(transId)) + amf0EncodeOne(cmdObj);
	if (!params.isNull()) {
		int len = params["length"].as<int>();
		for (int i = 0; i < len; i++) {
			data.append(amf0EncodeOne(params[i]));
		}
	}
	return data;
}
val amf0DecodeOne(MemoryStream& buffer);
val amf0decObject(MemoryStream& buffer) {
	val result = val::object();
	while (buffer[0] != AMF_END_OBJECT) {
		unsigned short len = buffer.readUInt16B();
		if (len > 0) {
			string name = buffer.readString(len);
			result.set(name, amf0DecodeOne(buffer));
		}
	}
	buffer>>=1;
	return result;
}
val amf0DecodeOne(MemoryStream& buffer) {
	if (buffer.length() == 0)return val::null();
	val result = val::null();
	byte type = buffer.readByte();
	switch (type) {
	case AMF_NUMBER:
		result = val(buffer.readDoubleB());
		break;
	case AMF_BOOLEAN:
		result = val(buffer.readB<1,bool>());
		break;
	case AMF_STRING:
		result = val(buffer.readString(buffer.readUInt16B()));
		break;
	case AMF_NULL:
		result = val::null();
		break;
	case AMF_UNDEFINED:
		result = val::undefined();
		break;
	case AMF_BEGIN_OBJECT:
		result = amf0decObject(buffer);
		break;
	case AMF_MIXED_ARRAY:
		break;
	}
	return result;
}
val decodeAmf0Cmd(MemoryStream& dbuf) {
	val resp = val::object();
	string cmd = amf0DecodeOne(dbuf).as<string>();
	resp.set("cmd", cmd);
	resp.set("transId", amf0DecodeOne(dbuf).as<int>());
	resp.set("cmdObj", amf0DecodeOne(dbuf));
	resp.set("params",val::array());
	int i = 0;
	while (dbuf.length()>0) {
		resp["params"].set(i++, amf0DecodeOne(dbuf));
	}
	return resp;
}