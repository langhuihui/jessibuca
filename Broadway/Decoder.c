#include "H264SwDecApi.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Debug prints */
#define DEBUG(argv) printf argv


u32 broadwayInit();
u32 broadwayDecode();
void broadwayExit();

const STREAM_BUFFER_SIZE = 1024 * 1024;
u8 *streamBuffer = NULL;

H264SwDecInst decInst;
H264SwDecInput decInput;
H264SwDecOutput decOutput;
H264SwDecPicture decPicture;
H264SwDecInfo decInfo;

u32 picDecodeNumber;
u32 picDisplayNumber;
u32 picSize;

typedef struct {
    u32 length;
    u8 *buffer;
    u8 *pos;
    u8 *end;
} Stream;

void streamInit(Stream *stream, u32 length) {
    stream->buffer = stream->pos = (u8 *)malloc(sizeof(u8) * length);
    stream->length = length;
    stream->end = stream->buffer + length;
}

void fillStreamWithFile(Stream *stream, char *filename) {
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        DEBUG(("UNABLE TO OPEN FILE\n"));
        return;
    }
    fseek(file, 0L, SEEK_END);
    u32 length = ftell(file);
    rewind(file);
    streamInit(stream, length);
    length = fread(stream->buffer, sizeof(u8), stream->length, file);
    fclose(file);
    if (length != stream->length) {
        DEBUG(("CANNOT READ FILE\n"));
    }
    DEBUG(("READ FILE %d BYTES\n", length));
}

void playStream(Stream *stream) {
    decInput.pStream = stream->buffer;
    decInput.dataLen = stream->length;
    u32 i = 0;
    do {
        u8 *start = decInput.pStream;
        u32 ret = broadwayDecode();
        // printf("Decoded Unit #%d, Size: %d, Result: %d\n", i++, (decInput.pStream - start), ret);
    } while (decInput.dataLen > 0);
}

void playFile(char *filename) {
    Stream file;
    fillStreamWithFile(&file, filename);
    broadwayInit();
    file.length = 10000;
    playStream(&file);
    // u8 sps[] = {103,66,192,30,146,68,5,1,127,203,8,0,0,31,72,0,7,83,0,120,177,117};
}


Stream broadwayStream;

u8 *broadwayCreateStream(u32 length) {
    streamInit(&broadwayStream, length);
    return broadwayStream.buffer;
}

void broadwaySetStreamLength(u32 length) {
    broadwayStream.length = length;
}

void broadwayPlayStream() {
    playStream(&broadwayStream);
}

/*
int main(int argc, char **argv) {
    playFile(argv[argc - 1]);
    return 0;
}
*/

u32 broadwayInit() {
    H264SwDecRet ret;
    u32 disableOutputReordering = 0;

    /* Initialize decoder instance. */
    ret = H264SwDecInit(&decInst, disableOutputReordering);
    if (ret != H264SWDEC_OK) {
        DEBUG(("DECODER INITIALIZATION FAILED\n"));
        broadwayExit();
        return -1;
    }

    picDecodeNumber = picDisplayNumber = 1;
}

extern void broadwayOnHeadersDecoded();

extern void broadwayOnPictureDecoded(u8 *buffer, u32 width, u32 height);

u32 broadwayDecode() {
    decInput.picId = picDecodeNumber;

    H264SwDecRet ret = H264SwDecDecode(decInst, &decInput, &decOutput);

    switch (ret) {
        case H264SWDEC_HDRS_RDY_BUFF_NOT_EMPTY:
            /* Stream headers were successfully decoded, thus stream information is available for query now. */
            ret = H264SwDecGetInfo(decInst, &decInfo);
            if (ret != H264SWDEC_OK) {
                return -1;
            }

            picSize = decInfo.picWidth * decInfo.picHeight;
            picSize = (3 * picSize) / 2;

            broadwayOnHeadersDecoded();

            decInput.dataLen -= decOutput.pStrmCurrPos - decInput.pStream;
            decInput.pStream = decOutput.pStrmCurrPos;
            break;

        case H264SWDEC_PIC_RDY_BUFF_NOT_EMPTY:
            /* Picture is ready and more data remains in the input buffer,
             * update input structure.
             */
            decInput.dataLen -= decOutput.pStrmCurrPos - decInput.pStream;
            decInput.pStream = decOutput.pStrmCurrPos;

            /* fall through */

        case H264SWDEC_PIC_RDY:
            if (ret == H264SWDEC_PIC_RDY) {
                decInput.dataLen = 0;
            }

            /* Increment decoding number for every decoded picture */
            picDecodeNumber++;

            while (H264SwDecNextPicture(decInst, &decPicture, 0) == H264SWDEC_PIC_RDY) {
                // printf(" Decoded Picture Decode: %d, Display: %d, Type: %s\n", picDecodeNumber, picDisplayNumber, decPicture.isIdrPicture ? "IDR" : "NON-IDR");

                /* Increment display number for every displayed picture */
                picDisplayNumber++;

                broadwayOnPictureDecoded(decPicture.pOutputPicture, decInfo.picWidth, decInfo.picHeight);
            }
            break;

        case H264SWDEC_STRM_PROCESSED:
        case H264SWDEC_STRM_ERR:
            /* Input stream was decoded but no picture is ready, thus get more data. */
            decInput.dataLen = 0;
            break;
    }
    return ret;
}

void broadwayExit() {
    if (streamBuffer) {
        free(streamBuffer);
    }
}

u8 *broadwayCreateStreamBuffer(u32 size) {
    u8 *buffer = (u8 *)malloc(sizeof(u8) * size);
    if (buffer == NULL) {
        DEBUG(("UNABLE TO ALLOCATE MEMORY\n"));
    }
    return buffer;
}

u32 broadwayGetMajorVersion() {
    return H264SwDecGetAPIVersion().major;
}

u32 broadwayGetMinorVersion() {
    return H264SwDecGetAPIVersion().minor;
}

/*------------------------------------------------------------------------------

    Function name:  H264SwDecTrace

    Purpose:
        Example implementation of H264SwDecTrace function. Prototype of this
        function is given in H264SwDecApi.h. This implementation appends
        trace messages to file named 'dec_api.trc'.

------------------------------------------------------------------------------*/
void H264SwDecTrace(char *string)
{
    printf("%s\n", string);
}

/*------------------------------------------------------------------------------

    Function name:  H264SwDecMalloc

    Purpose:
        Example implementation of H264SwDecMalloc function. Prototype of this
        function is given in H264SwDecApi.h. This implementation uses
        library function malloc for allocation of memory.

------------------------------------------------------------------------------*/
void* H264SwDecMalloc(u32 size)
{

#if defined(CHECK_MEMORY_USAGE)
    /* Note that if the decoder has to free and reallocate some of the buffers
     * the total value will be invalid */
    static u32 numBytes = 0;
    numBytes += size;
    DEBUG(("Allocated %d bytes, total %d\n", size, numBytes));
#endif

    return malloc(size);
}

/*------------------------------------------------------------------------------

    Function name:  H264SwDecFree

    Purpose:
        Example implementation of H264SwDecFree function. Prototype of this
        function is given in H264SwDecApi.h. This implementation uses
        library function free for freeing of memory.

------------------------------------------------------------------------------*/
void H264SwDecFree(void *ptr)
{
    free(ptr);
}

/*------------------------------------------------------------------------------

    Function name:  H264SwDecMemcpy

    Purpose:
        Example implementation of H264SwDecMemcpy function. Prototype of this
        function is given in H264SwDecApi.h. This implementation uses
        library function memcpy to copy src to dest.

------------------------------------------------------------------------------*/
void H264SwDecMemcpy(void *dest, void *src, u32 count)
{
    memcpy(dest, src, count);
}

/*------------------------------------------------------------------------------

    Function name:  H264SwDecMemset

    Purpose:
        Example implementation of H264SwDecMemset function. Prototype of this
        function is given in H264SwDecApi.h. This implementation uses
        library function memset to set content of memory area pointed by ptr.

------------------------------------------------------------------------------*/
void H264SwDecMemset(void *ptr, i32 value, u32 count)
{
    memset(ptr, value, count);
}
