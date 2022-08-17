#ifndef _AV_TYPE_H_
#define _AV_TYPE_H_

enum VideoType {

    Video_Unknow = 0,
    Video_H264   = 0x01,
    Video_H265   = 0x02,
    Video_AV1    = 0x04

};

enum VideoFormatType {

    Format_AVCC         = 0x01,
    Format_AVC_AnnexB   = 0x02,
    Format_HVCC         = 0x03,
    Format_HEVC_AnnexB  = 0x04

};

enum AudioType {

    Audio_Unknow = 0,
    Audio_PCMA   = 0x1,
    Audio_PCMU   = 0x2,
    Audio_AAC    = 0x4,
    Audio_OPUS   = 0x8

};



#endif