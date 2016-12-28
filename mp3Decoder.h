extern "C"
{
    int decode_header(struct mad_header * header, struct mad_stream * stream);
    int free_bitrate(struct mad_stream * stream, struct mad_header const *header);
    struct sideinfo
    {
        unsigned int main_data_begin;
        unsigned int private_bits;

        unsigned char scfsi[2];

        struct granule
        {
            struct channel
            {
                /* from side info */
                unsigned short part2_3_length;
                unsigned short big_values;
                unsigned short global_gain;
                unsigned short scalefac_compress;

                unsigned char flags;
                unsigned char block_type;
                unsigned char table_select[3];
                unsigned char subblock_gain[3];
                unsigned char region0_count;
                unsigned char region1_count;

                /* from main_data */
                unsigned char scalefac[39]; /* scalefac_l and/or scalefac_s */
            } ch[2];
        } gr[2];
    };
    enum mad_error III_sideinfo(struct mad_bitptr * ptr, unsigned int nch,
                                       int lsf, struct sideinfo *si,
                                       unsigned int *data_bitlen,
                                       unsigned int *priv_bitlen);
    enum mad_error III_decode(struct mad_bitptr * ptr, struct mad_frame * frame,
                                     struct sideinfo * si, unsigned int nch);
}

class MP3Decoder
{
    mad_stream inputStream;
    mad_frame frame;
    mad_synth synth;
    MemoryStream lastInput;
    MemoryStream mainBuffer;
  public:
    MP3Decoder()
    {
        mad_stream_init(&inputStream);
        mad_frame_init(&frame);
        mad_synth_init(&synth);
        emscripten_log(0, "mp3 init!");
    }
    ~MP3Decoder()
    {
        mad_synth_finish(&synth);
        mad_frame_finish(&frame);
    }
    int decodeHeader(unsigned const char *headerPtr)
    {
        unsigned int pad_slot, N;
        struct mad_header &header = frame.header;
        mad_bit_init(&inputStream.ptr, headerPtr);
        inputStream.this_frame = headerPtr;
        decode_header(&header, &inputStream);
        
        /* calculate frame duration */
        mad_timer_set(&header.duration, 0,
                      32 * MAD_NSBSAMPLES(&header), header.samplerate);
        /* calculate free bit rate */
        if (header.bitrate == 0)
        {
            
            if ((inputStream.freerate == 0 ||
                 (header.layer == MAD_LAYER_III && inputStream.freerate > 640000)) &&
                free_bitrate(&inputStream, &header) == -1)
                return -1;
            
            header.bitrate = inputStream.freerate;
            header.flags |= MAD_FLAG_FREEFORMAT;
        }
        /* calculate beginning of next frame */
        pad_slot = (header.flags & MAD_FLAG_PADDING) ? 1 : 0;
        if (header.layer == MAD_LAYER_I)
            N = ((12 * header.bitrate / header.samplerate) + pad_slot) * 4;
        else
        {
            unsigned int slots_per_frame;

            slots_per_frame = (header.layer == MAD_LAYER_III &&
                               (header.flags & MAD_FLAG_LSF_EXT))
                                  ? 72
                                  : 144;

            N = (slots_per_frame * header.bitrate / header.samplerate) + pad_slot;
        }
        emscripten_log(0, "audio size:%d bitrate:%d samplerate:%d",N ,header.bitrate,header.samplerate);
        return N;
    }
    int checkCRC()
    {
        struct mad_header &header = frame.header;
        // if (mainStream.main_data == 0)
        // {
        //     mainStream.main_data = malloc(MAD_BUFFER_MDLEN);
        //     if (mainStream.main_data == 0)
        //     {
        //         mainStream.error = MAD_ERROR_NOMEM;
        //         return -1;
        //     }
        // }

        if (frame.overlap == 0)
        {
            frame.overlap = (decltype(frame.overlap))calloc(2 * 32 * 18, sizeof(mad_fixed_t));
            if (frame.overlap == 0)
            {
                inputStream.error = MAD_ERROR_NOMEM;
                return -1;
            }
        }
        auto nch = MAD_NCHANNELS(&header);
        auto si_len = (header.flags & MAD_FLAG_LSF_EXT) ? (nch == 1 ? 9 : 17) : (nch == 1 ? 17 : 32);

        if (header.flags & MAD_FLAG_PROTECTION)
        {
            header.crc_check =
                mad_bit_crc(inputStream.ptr, si_len * CHAR_BIT, header.crc_check);

            if (header.crc_check != header.crc_target &&
                !(frame.options & MAD_OPTION_IGNORECRC))
            {
                inputStream.error = MAD_ERROR_BADCRC;
                return -1;
            }
        }
        return si_len;
    }
    int get_main_data_begin(unsigned const char *ptr)
    {
        struct mad_bitptr peek;
        unsigned long header;
        int md_begin = 0;
        mad_bit_init(&peek, ptr);

        header = mad_bit_read(&peek, 32);
        if ((header & 0xffe60000L) /* syncword | layer */ == 0xffe20000L)
        {
            if (!(header & 0x00010000L)) /* protection_bit */
                mad_bit_skip(&peek, 16); /* crc_check */

            md_begin =
                mad_bit_read(&peek, (header & 0x00080000L) /* ID */ ? 9 : 8);
        }

        mad_bit_finish(&peek);
        return md_begin;
    }
    int decode()
    {
        auto startPtr = (unsigned const char *)lastInput.point();
        int size = decodeHeader(startPtr);
        if(size==-1)return -1;
        lastInput >>= size;
        if (lastInput.length() == 0)
        {
            lastInput<<=size;
            return -1;
        }
        auto si_len = checkCRC();
        struct sideinfo si;
        unsigned int priv_bitlen, data_bitlen, next_md_begin = 0;
        auto nch = MAD_NCHANNELS(&frame.header);
        auto error = III_sideinfo(&inputStream.ptr, nch, frame.header.flags & MAD_FLAG_LSF_EXT,
                                  &si, &data_bitlen, &priv_bitlen);
        frame.header.flags |= priv_bitlen;
        frame.header.private_bits |= si.private_bits;
        auto main_begin = mad_bit_nextbyte(&inputStream.ptr);
        auto headerSize = main_begin - startPtr;
        int mainSize = size - headerSize;
        /* find main_data of next frame */
        next_md_begin = get_main_data_begin((const unsigned char *)lastInput.point());
        int oldSize = mainBuffer.length();
        mainBuffer << MemoryStream((u8 *)main_begin, mainSize);
        lastInput.removeConsume();
        int newSize = mainBuffer.length();
        if (oldSize < si.main_data_begin || newSize < next_md_begin)
            return -1;
        mainBuffer.offset = oldSize - si.main_data_begin;
        int dataSize = newSize - next_md_begin - mainBuffer.offset;
        struct mad_bitptr mainStreamBitPtr;
        mad_bit_init(&mainStreamBitPtr, (unsigned const char *)mainBuffer.point());
        III_decode(&mainStreamBitPtr, &frame, &si, nch);
        mad_synth_frame(&synth, &frame);
        mainBuffer >>= dataSize;
        mainBuffer.removeConsume();
        return 0;
    }
    int decode(MemoryStream &input)
    {
        //frame.options = inputStream.options;
        int oldSize=lastInput.size();
        lastInput.data.resize(lastInput.size()+input.length());
        memcpy((void*)(lastInput.data.data()+oldSize), input.point(), input.length());
        return decode();
    }

    mad_pcm &getPCM()
    {
        return synth.pcm;
    }
};
