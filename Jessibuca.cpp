#ifdef EDITTIME
#undef __cplusplus
#define __cplusplus 201703L
#endif
#include "base.h"
#include <regex>
#define PROP(name, type)         \
    type name;                   \
    val get##name() const        \
    {                            \
        return val(name);        \
    }                            \
    void set##name(val value)    \
    {                            \
        name = value.as<type>(); \
    }
extern "C"
{
    extern void init(void);
}
int main()
{
    init();
    return 0;
}

struct Jessica
{
    val wrapped;
    bool flvHeadRead;
    IOBuffer buffer;
    AUDIO_DECODER audioDecoder;
    VIDEO_DECODER videoDecoder;
    queue<VideoPacket> videoBuffers;
    bool bufferIsPlaying = false;
    int videoTimeoutId = 0;
    bool waitFirstVideo = true;
    bool waitFirstAudio = true;
    clock_t lastDataTime = 0;
    clock_t lastVideoTimeStamp = 0;
    int bytesCount = 0;
    PROP(isPlaying, bool)
    PROP(flvMode, bool)
    PROP(audioBuffer, int)
    PROP(videoBuffer, int)
    PROP(bps, double)
    Jessica(val &&v) : wrapped(forward<val>(v)), isPlaying(false), flvMode(false), flvHeadRead(false), audioBuffer(12)
    {
        videoDecoder.jsObject = &wrapped;
    }
    template <typename... Args>
    Jessica(Args &&... args) : wrapped(val::undefined())
    {
    }
    virtual ~Jessica()
    {
        val::global("clearTimeout")(videoTimeoutId);
        emscripten_log(0, "FlvDecoder release!\n");
    }

    void $play(const string &url)
    {
        if (isPlaying)
        {
            call<void>("close");
        }
        smatch urlm;
        regex_match(url, urlm, regex("wss?://(.+)(:\\d+)(/.+)?"));
        if (!regex_match(urlm.str(1),  regex("(demo\\.monibuca\\.com|localhost|[\\d\\.]+)"))){
            emscripten_log(1, "%s Unauthorized",urlm.str(1).c_str());
            return;
        }
        isPlaying = true;
        bool webgl = wrapped["isWebGL"].as<bool>();
        emscripten_log(0, "webgl:%s", webgl ? "true" : "false");
        videoDecoder.webgl = webgl;
        flvMode = url.find(".flv") != string::npos;
        // #define WS_PREFIX "ws://test.qihaipi.com/gnddragon/"
#ifdef WS_PREFIX
        val ws = val::global("WebSocket").new_(WS_PREFIX + url);
#else
        val ws = val::global("WebSocket").new_(url);
#endif
        ws.set("binaryType", "arraybuffer");
        ws.set("onmessage", bind("onData"));
        lastDataTime = clock();
        // ws.set("onerror", bind("onError"));
        wrapped.set("ws", ws);
    }
    void onData(val evt)
    {
        string data = evt["data"].as<string>();
        bytesCount += data.length();
        auto now = clock();
        if (now > lastDataTime)
        {
            bps = bytesCount * 1000.0 / (now - lastDataTime);
            lastDataTime = now;
            bytesCount = 0;
        }
        if (flvMode)
        {
            buffer << move(data);
            if (!flvHeadRead)
            {
                if (buffer.length >= 13)
                {
                    flvHeadRead = true;
                    buffer >>= 13;
                }
            }
            else
            {
                while (buffer.length > 3)
                {
                    u8 type = buffer.readu8();
                    unsigned int length = buffer.readUInt24B();
                    if (buffer.length < length + 4 + 7)
                    {
                        buffer <<= 4;
                        break;
                    }
                    unsigned int timestamp = buffer.readUInt24B();
                    //u8 ext = buffer.readu8();
                    // buffer.readUInt24B();
                    buffer >>= 4;
                    IOBuffer payload;
                    payload << IOBuffer(&buffer, 0, length);
                    switch (type)
                    {
                    case 0x08:
                        decodeAudio(timestamp, payload);
                        break;
                    case 0x09:
                        decodeVideo(timestamp, payload);
                        break;
                    default:
                        emscripten_log(0, "unknow type: %d", type);
                        break;
                    }
                    buffer >>= length;
                    length = buffer.readUInt32B();
                }
                buffer.removeConsume();
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
#ifdef USE_FFMPEG
    void decodeAudio(clock_t timestamp, IOBuffer ms)
    {
        unsigned char flag = 0;
        ms.readB<1>(flag);
        int bytesCount = audioDecoder.decode(ms);
        if(!bytesCount)return;
        if (!waitFirstAudio)
        {
            call<void>("playAudioPlanar", int(audioDecoder.frame->data), bytesCount);
        }
        else
        {
            call<void>("initAudioPlanar", audioDecoder.dec_ctx->channels, audioDecoder.dec_ctx->sample_rate);
            waitFirstAudio = false;
        }
    }
#else
    void decodeAudio(clock_t timestamp, IOBuffer ms)
    {
        if (ms[0] == 0xFF && (ms[1] & 0xF0) == 0xF0)
        {
            //ADTS 头
            call<void>("playAudio", int(ms.point()), ms.length);
            return;
        }
        unsigned char flag = 0;
        ms.readB<1>(flag);
        auto audioType = flag >> 4;
        if (waitFirstAudio)
        {
            int channels = (flag & 1) + 1;
            int rate = (flag >> 2) & 3;
            switch (rate)
            {
            case 1:
                rate = 11025;
                break;
            case 2:
                rate = 22050;
                break;
            case 3:
                rate = 44100;
                break;
            }
            switch (audioType)
            {
            case 10: //AAC
                // initAudio(audioBuffer * 1024, rate, channels);
                audioDecoder.decode(audioType, ms);
                initAudio(audioBuffer * 1024, audioDecoder.samplerate, audioDecoder.channels);
                return;
            case 11: //Speex
                initAudio(50 * 320, 16000, channels);
                break;
            case 2: //MP3
                initAudio(audioBuffer * 576, rate, channels);
                break;
            }
        }
        if (!waitFirstAudio && audioDecoder.decode(audioType, ms))
            call<void>("playAudio");
    }
    void initAudio(int frameCount, int samplerate, int channels)
    {
        waitFirstAudio = false;
        audioDecoder.init(frameCount * channels * 2);
        call<void>("initAudio", frameCount, samplerate, channels, (int)audioDecoder.outputBuffer >> 1);
    }
#endif
    void decodeVideo(clock_t _timestamp, IOBuffer data)
    {
        if (waitFirstVideo)
        {
            if (videoDecoder.isAVCSequence(data))
            {
                videoDecoder.decode(data);
                waitFirstVideo = false;
                emscripten_log(0, "video info set!");
            }
        }
        else if (data[1] == 1 || data[1] == 0)
        {
            if (_timestamp == 0 && lastVideoTimeStamp != 0)
                return;
            if (videoBuffer && (bufferIsPlaying || checkTimeout(_timestamp)))
            {
                videoBuffers.emplace(_timestamp, data);
                //emscripten_log(0, "push timestamp:%d", _timestamp);
                // auto &&info = val::object();
                // info.set("code", "NetStream.Play.Start");
                // call<void>("onNetStatus", info);
            }
            else
            {
                videoDecoder.decode(data);
            }
        }
        else
        {
            call<void>("resetTimeSpan");
        }
        lastVideoTimeStamp = _timestamp;
    }
    void decodeVideoBuffer()
    {
        bool check = false;
        while (!videoBuffers.empty())
        {
            auto &v = videoBuffers.front();
            if (check && checkTimeout(v.timestamp))
                return;
            //emscripten_log(0, "play timestamp:%d", v.timestamp);
            videoDecoder.decode(v.data);
            videoBuffers.pop();
            check = true;
        }
        bufferIsPlaying = false;
    }
    bool checkTimeout(clock_t timestamp)
    {
        auto timeout = getTimespan(timestamp);
        bool isTimeout = timeout > 0;
        if (isTimeout)
        {
            bufferIsPlaying = true;
            videoTimeoutId = call<int>("playVideoBuffer", timeout);
        }
        return isTimeout;
    }
    val getBufferInfo() const
    {
        val &&result = val::object();
        result.set("front", videoBuffers.front().timestamp);
        result.set("back", videoBuffers.back().timestamp);
        result.set("size", videoBuffers.size());
        return result;
    }
    clock_t getTimespan(clock_t t)
    {
        // if (videoBuffers.size() > 4)
        // {
        //     videoBuffer = videoBuffer >> 1;
        // }
        return call<clock_t>("timespan", t) + videoBuffer;
    }
    void $close()
    {
        val::global("clearTimeout")(videoTimeoutId);
        videoBuffers = queue<VideoPacket>();
        // while (!videoBuffers.empty())
        // {
        //     videoBuffers.pop();
        // }
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
    ReturnType call(const char *name, Args &&... args) const
    {
        return wrapped.call<ReturnType>(name, forward<Args>(args)...);
    }
};

struct Jessibuca : public wrapper<Jessica>
{
    EMSCRIPTEN_WRAPPER(Jessibuca)
};
#define FUNC(name) function(#name, &Jessica::name)
#undef PROP
#define PROP(name) property(#name, &Jessica::get##name, &Jessica::set##name)
EMSCRIPTEN_BINDINGS(Jessica)
{
    class_<Jessica>("Jessica")
        .FUNC($play)
        .FUNC(onData)
        .FUNC($close)
        .FUNC(decodeVideoBuffer)
        .PROP(isPlaying)
        .PROP(flvMode)
        .PROP(audioBuffer)
        .PROP(videoBuffer)
        .PROP(bps)
        .property("bufferInfo", &Jessica::getBufferInfo)
        // .function("invoke", &H5LCBase::invoke, pure_virtual())
        .allow_subclass<Jessibuca, val>("Jessibuca", constructor<val>());
}