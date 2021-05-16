#!/usr/bin/python

import os

args = [
    'extra-cflags="-c -Oz -fPIC -pthread"',
    'cc="emcc"',
    'cxx="em++"',
    'ar="emar"',
    'prefix=../obj',
    'target-os=none',
    'arch=x86_32',
    'cpu=generic'
    ]
disables = [
    'everything','all','avdevice','avformat','postproc','avfilter','programs','asm','parsers','muxers','demuxers','filters',
    'doc','devices','network','bsfs','shared','hwaccels','debug','protocols','indevs','outdevs','runtime-cpudetect'
]
enables = [
    'small','cross-compile','gpl','avcodec','swresample',
    'parser=h264',
    'parser=hevc',
    'parser=aac',
    'decoder=h264',
    'decoder=hevc',
    'decoder=aac',
    'decoder=pcm_alaw',
    'decoder=pcm_mulaw',
    'encoder=pcm_alaw',
    'encoder=pcm_mulaw',
    'encoder=aac','static'
]
enables = ['enable-'+item for i,item in enumerate(enables)]
disables = ['disable-'+item for i,item in enumerate(disables)]
os.system('emconfigure ./configure --' +
          (' --'.join(args+disables+enables)))
os.system('make clean && emmake make && make install')