#ifdef EDITTIME
#undef __cplusplus
#define __cplusplus 201703L
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <functional>
#include <map>
#include <queue>
#include <emscripten.h>
#include <emscripten/bind.h>
#include <emscripten/val.h>
#include <time.h>
// #include <regex>
using namespace std;
using namespace emscripten;
#include "slice.h"
#include "ffmpeg.h"
#define PROP(name, type)                        \
    type name;                                  \
    val get##name() const                       \
    {                                           \
        return val(name);                       \
    }                                           \
    void set##name(val value)                   \
    {                                           \
        name = value.as<type>();                \
        emscripten_log(0, #name " = %d", name); \
    }

struct Jessica
{
    val wrapped;
    bool flvHeadRead;
    string buffer;
    FFmpegAudioDecoder audioDecoder;
    FFmpegVideoDecoder videoDecoder;
    queue<VideoPacket> videoBuffers;
    bool bufferIsPlaying = false;
    int videoTimeoutId = 0;
    bool waitFirstVideo = true;
    bool waitFirstAudio = true;
    clock_t lastDataTime = 0;
    clock_t lastVideoTimeStamp = 0;
    clock_t firstVideoTimeStamp = 0;
    clock_t firstTimeStamp = 0;
    int idr_count = 0; //是否正在丢帧
    int bytesCount = 0;
    int delay = 0;
    PROP(flvMode, bool)
    PROP(audioBuffer, int)
    PROP(videoBuffer, int)
    PROP(bps, double)
    Jessica(val &&v) : wrapped(forward<val>(v)),flvMode(false), flvHeadRead(false), audioBuffer(12)
    {
        videoDecoder.jsObject = &wrapped;
    }
    template <typename... Args>
    Jessica(Args &&...args) : wrapped(val::undefined())
    {
    }
    virtual ~Jessica()
    {
        val::global("clearTimeout")(videoTimeoutId);
        emscripten_log(0, "FlvDecoder release!\n");
    }

    void onData(val evt)
    {
        string data = evt.as<string>();
        bytesCount += data.length();
        auto now = clock();
        if (now > lastDataTime)
        {
            bps = bytesCount * 1000000.0 / (now - lastDataTime);
            lastDataTime = now;
            bytesCount = 0;
        }
        if (flvMode)
        {
            buffer.append(data);
            if (!flvHeadRead)
            {
                if (buffer.length() >= 13)
                {
                    flvHeadRead = true;
                    buffer = buffer.substr(13);
                }
            }
            else
            {
                while (buffer.length() > 3)
                {
                    size_t length = 0;
                    auto attr = (u8 *)(&length);
                    attr[2] = buffer[1];
                    attr[1] = buffer[2];
                    attr[0] = buffer[3];
                    if (buffer.length() < length + 15)
                    {
                        break;
                    }
                    unsigned int timestamp = 0;
                    attr = (u8 *)(&timestamp);
                    attr[2] = buffer[4];
                    attr[1] = buffer[5];
                    attr[0] = buffer[6];
                    IOBuffer payload(buffer.substr(11, length));
                    switch (buffer[0])
                    {
                    case 8:
                        decodeAudio(timestamp, payload);
                        break;
                    case 9:
                        decodeVideo(timestamp, payload);
                        break;
                    default:
                        emscripten_log(0, "unknow type: %d", buffer[0]);
                        break;
                    }
                    buffer = buffer.substr(length + 15);
                }
            }
        }
        else
        {
            switch (data.at(0))
            {
            case 1:
            {
                IOBuffer b(move(data));
                b >>= 1;
                decodeAudio(b.readUInt32B(), b);
            }
            break;
            case 2:
            {
                IOBuffer b(move(data));
                b >>= 1;
                decodeVideo(b.readUInt32B(), b);
            }
            break;
            case 10:
            {
                wrapped["ws"].call<void>("send", val("[\"__bandwidth\"]"));
            }
            break;
            default:
                emscripten_log(1, "error type :%d", data.at(0));
                break;
            }
        }
    }
    void decodeAudio(clock_t timestamp, IOBuffer ms)
    {
        unsigned char flag = 0;
        ms.readB<1>(flag);
        unsigned char audioType = flag >> 4;
        int bytesCount = audioDecoder.decode(ms);
        if (!bytesCount)
            return;
        if (!waitFirstAudio)
        {
            call<void>("playAudioPlanar", int(audioDecoder.frame->data), bytesCount, timestamp);
        }
        else
        {
            call<void>("initAudioPlanar", audioDecoder.dec_ctx->channels, audioDecoder.dec_ctx->sample_rate);
            waitFirstAudio = false;
        }
    }
    // void decodeAudio(clock_t timestamp, IOBuffer ms)
    // {
    //     if (ms[0] == 0xFF && (ms[1] & 0xF0) == 0xF0)
    //     {
    //         //ADTS 头
    //         call<void>("playAudio", int(ms.point()), ms.length);
    //         return;
    //     }
    //     unsigned char flag = 0;
    //     ms.readB<1>(flag);
    //     auto audioType = flag >> 4;
    //     if (waitFirstAudio)
    //     {
    //         int channels = (flag & 1) + 1;
    //         int rate = (flag >> 2) & 3;
    //         switch (rate)
    //         {
    //         case 1:
    //             rate = 11025;
    //             break;
    //         case 2:
    //             rate = 22050;
    //             break;
    //         case 3:
    //             rate = 44100;
    //             break;
    //         }
    //         switch (audioType)
    //         {
    //         case 10: //AAC
    //             // initAudio(audioBuffer * 1024, rate, channels);
    //             audioDecoder.decode(ms);
    //             initAudio(audioBuffer * 1024, audioDecoder.samplerate, audioDecoder.channels);
    //             return;
    //         case 11: //Speex
    //             initAudio(50 * 320, 16000, channels);
    //             break;
    //         case 2: //MP3
    //             initAudio(audioBuffer * 576, rate, channels);
    //             break;
    //         }
    //     }
    //     if (!waitFirstAudio && audioDecoder.decode(ms))
    //         call<void>("playAudio", timestamp);
    // }
    // void initAudio(int frameCount, int samplerate, int channels)
    // {
    //     waitFirstAudio = false;
    //     audioDecoder.init(frameCount * channels * 2);
    //     call<void>("initAudio", frameCount, samplerate, channels, (int)audioDecoder.outputBuffer >> 1);
    // }
    void decodeVideo(clock_t _timestamp, IOBuffer data)
    {
        if (waitFirstVideo)
        {
            if (videoDecoder.isAVCSequence(data))
            {
                videoDecoder.decode(data);
                waitFirstVideo = false;
                emscripten_log(0, "video info set! video buffer: %dms", videoBuffer);
            }
        }
        else if (data[1] == 1 || data[1] == 0)
        {
            if (_timestamp == 0 && lastVideoTimeStamp != 0)
                return;
            delay = call<int>("getDelay", int(_timestamp));
            if (videoBuffer == 0)
            {
                if (delay > 3000)
                {
                    emscripten_log(0, "delay: %dms", delay);
                    return;
                }
                videoDecoder.timestamp = _timestamp;
                videoDecoder.decode(data);
            }
            else
            {
                videoBuffers.emplace(_timestamp, data);
                auto &v = videoBuffers.front();
                if (delay + (lastVideoTimeStamp - v.timestamp) > videoBuffer)
                {
                    if (!bufferIsPlaying)
                    {
                        bufferIsPlaying = true;
                        decodeVideoBuffer();
                    }
                }
            }
        }
        lastVideoTimeStamp = _timestamp;
    }

    void decodeVideoBuffer()
    {
        if (videoBuffers.size() > 1)
        {
            auto &v = videoBuffers.front();
            videoDecoder.timestamp = v.timestamp;
            auto start = clock();
            videoDecoder.decode(v.data);
            auto cost = (clock() - start) / 1000;
            videoBuffers.pop();
            auto timeout = videoBuffers.front().timestamp - v.timestamp;
            if (timeout > cost)
            {
                timeout = timeout - cost;
            }
            // if (idr_count > 5)
            // {
            //     while (videoBuffers.size() > 0 && !videoBuffers.front().isKeyFrame)
            //     {
            //         videoBuffers.pop();
            //     }
            //     decodeVideoBuffer();
            //     return;
            // }
            if (delay + (lastVideoTimeStamp - v.timestamp) > videoBuffer)
            {
                decodeVideoBuffer();
            }
            else
                val::global("setTimeout")(bind("decodeVideoBuffer"), timeout);
            return;
        }
        bufferIsPlaying = false;
    }
    val getBufferInfo() const
    {
        val &&result = val::object();
        result.set("front", videoBuffers.front().timestamp);
        result.set("back", videoBuffers.back().timestamp);
        result.set("size", videoBuffers.size());
        return result;
    }
    void $close()
    {
        val::global("clearTimeout")(videoTimeoutId);
        videoBuffers = queue<VideoPacket>();
        videoDecoder.clear();
        audioDecoder.clear();
        videoTimeoutId = 0;
        waitFirstVideo = true;
        waitFirstAudio = true;
        bufferIsPlaying = false;
        lastVideoTimeStamp = 0;
        buffer.clear();
        flvHeadRead = false;
    }
    val bind(const char *name)
    {
        return wrapped[name].call<val>("bind", wrapped);
    }
    template <typename ReturnType, typename... Args>
    ReturnType call(const char *name, Args &&...args) const
    {
        return wrapped.call<ReturnType>(name, forward<Args>(args)...);
    }
};

#define FUNC(name) function(#name, &Jessica::name)
#undef PROP
#define PROP(name) property(#name, &Jessica::get##name, &Jessica::set##name)
EMSCRIPTEN_BINDINGS(Jessica)
{
    class_<Jessica>("Jessica")
        .FUNC(onData)
        .FUNC($close)
        .FUNC(decodeVideoBuffer)
        .PROP(flvMode)
        .PROP(audioBuffer)
        .PROP(videoBuffer)
        .PROP(bps)
        .property("bufferInfo", &Jessica::getBufferInfo);
}