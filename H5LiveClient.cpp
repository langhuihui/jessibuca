#ifdef EDITTIME
#undef __cplusplus
#define __cplusplus 201703L
#endif
#include "base.h"
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

struct H5LCBase
{
    val wrapped;
    bool flvHeadRead;
    MemoryStream buffer;
    AudioDecoder audioDecoder;
    VIDEO_DECODER videoDecoder;
    queue<VideoPacket> videoBuffers;
    bool bufferIsPlaying = false;
    int videoTimeoutId = 0;
    bool waitFirstVideo = true;
    bool waitFirstAudio = true;
    clock_t lastDataTime = 0;
    int bytesCount = 0;
    PROP(isPlaying, bool)
    PROP(flvMode, bool)
    PROP(audioBuffer, int)
    PROP(videoBuffer, int)
    PROP(bps, double)
    H5LCBase(val &&v) : wrapped(forward<val>(v)), isPlaying(false), flvMode(false), flvHeadRead(false), audioBuffer(12)
    {
        videoDecoder.jsObject = &wrapped;
    }
    template <typename... Args>
    H5LCBase(Args &&... args) : wrapped(val::undefined())
    {
    }
    virtual ~H5LCBase()
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
        isPlaying = true;
        bool webgl = wrapped["webGLCanvas"].call<bool>("isWebGL");
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
            buffer << data;
            if (!flvHeadRead)
            {
                if (buffer.length() >= 13)
                {
                    flvHeadRead = true;
                    buffer.offset = 13;
                    buffer.removeConsume();
                }
            }
            else
            {
                while (buffer.length() > 3)
                {
                    u8 type = buffer.readu8();
                    unsigned int length = buffer.readUInt24B();
                    if (buffer.length() < length + 4 + 7)
                    {
                        buffer <<= 4;
                        break;
                    }
                    unsigned int timestamp = buffer.readUInt24B();
                    u8 ext = buffer.readu8();
                    buffer.readUInt24B();
                    MemoryStream ms;
                    ms << buffer.readString(length);
                    switch (type)
                    {
                    case 0x08:
                        decodeAudio(timestamp, move(ms));
                        break;
                    case 0x09:
                        decodeVideo(timestamp, move(ms));
                        break;
                    default:
                        emscripten_log(0, "unknow type: %d", type);
                        break;
                    }
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
                MemoryStream ms(data.substr(1));
                decodeAudio(ms.readUInt32B(), move(ms));
            }
            break;
            case 2:
            {
                MemoryStream ms(data.substr(1));
                decodeVideo(ms.readUInt32B(), move(ms));
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
    void decodeAudio(clock_t timestamp, MemoryStream &&ms)
    {
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
                initAudio(audioBuffer * 1024, rate, channels);
                break;
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
    void decodeVideo(clock_t _timestamp, MemoryStream &&data)
    {
        u8 avc_packet_type = data[1]; //0为AVCSequence Header，1为AVC NALU，2为AVC end ofsequence
        if (waitFirstVideo)
        {
            u8 frame_type = data[0];
            int codec_id = frame_type & 0x0f;
            frame_type = (frame_type >> 4) & 0x0f;
            if (codec_id == 7)
            {
                emscripten_log(0, "got h264 video");
            }
            else if (codec_id == 12)
            {
                emscripten_log(0, "got h265 video");
            }
            else
            {
                emscripten_log(0, "Only support video h.264/avc or h.265/hevc codec. actual=%d", codec_id);
                return;
            }

            if (frame_type == 1 && avc_packet_type == 0)
            {
                videoDecoder.decodeHeader(data, codec_id);
                waitFirstVideo = false;
                emscripten_log(0, "video info set!");
            }
        }
        else if (avc_packet_type == 1)
        {
            if (_timestamp == 0)
                return;
            data >>= 5;
            if (videoBuffer && (bufferIsPlaying || checkTimeout(_timestamp)))
            {
                videoBuffers.emplace(_timestamp, forward<MemoryStream>(data));
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
        return call<clock_t>("timespan", t) + videoBuffer * 1000;
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
        bufferIsPlaying = false;
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

struct H5LiveClient : public wrapper<H5LCBase>
{
    EMSCRIPTEN_WRAPPER(H5LiveClient)
};
#define FUNC(name) function(#name, &H5LCBase::name)
#undef PROP
#define PROP(name) property(#name, &H5LCBase::get##name, &H5LCBase::set##name)
EMSCRIPTEN_BINDINGS(H5LCBase)
{
    class_<H5LCBase>("H5LCBase")
        .FUNC($play)
        .FUNC(onData)
        .FUNC($close)
        .FUNC(decodeVideoBuffer)
        .PROP(isPlaying)
        .PROP(flvMode)
        .PROP(audioBuffer)
        .PROP(videoBuffer)
        .PROP(bps)
        .property("bufferInfo", &H5LCBase::getBufferInfo)
        // .function("invoke", &H5LCBase::invoke, pure_virtual())
        .allow_subclass<H5LiveClient, val>("H5LiveClient", constructor<val>());
}