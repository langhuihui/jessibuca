class MP3Decoder
{
    mad_stream stream;
    mad_frame frame;
    mad_synth synth;
    MemoryStream mainData;
    MP3Decoder()
    {
        mad_stream_init(&stream);
        mad_frame_init(&frame);
        mad_synth_init(&synth);
        emscripten_log(0, "mp3 init!");
    }
    ~MP3Decoder()
    {
        mad_synth_finish(&synth);
        mad_frame_finish(&frame);
        mad_stream_finish(&stream);
    }
    int decodeHeader(const char *headerPtr)
    {
        register unsigned char const *ptr, *end;
        unsigned int pad_slot, N;
        mad_bit_init(&stream.ptr, headerPtr);
        ptr = mad_bit_nextbyte(&stream.ptr);
        if (!(ptr[0] == 0xff && (ptr[1] & 0xe0) == 0xe0))
            return -1;
        mad_bit_init(&stream.ptr, ptr);
        ptr = mad_bit_nextbyte(&stream.ptr);
        stream.this_frame = ptr;
        mad_bit_init(&stream.ptr, stream.this_frame);
        decode_header(&frame.header, &stream);
        return -1;
    }
    mad_pcm &decode(MemoryStream &input)
    {
        frame.options = stream.options;
        decodeHeader(input.point());
    }
}
