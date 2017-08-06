#!/usr/bin/python

import os, sys, re, json, shutil
from subprocess import Popen, PIPE, STDOUT

exec(open(os.path.expanduser('~/.emscripten'), 'r').read())

sys.path.append(EMSCRIPTEN_ROOT)
import tools.shared as emscripten

emcc_args = [
'--pre-js',os.path.join('Broadway','WebGLCanvas.js'),
  #'-m32',
  '-O3',
 '--memory-init-file', '0',
  '--llvm-opts', '3',
  #'-s', 'CORRECT_SIGNS=1',
  #'-s', 'CORRECT_OVERFLOWS=1',
 '-s', 'TOTAL_MEMORY=67108864',
  #'-s', 'FAST_MEMORY=' + str(12*1024*1024),
  #'-s', 'INVOKE_RUN=0',
  '-s', 'ASSERTIONS=1', 
 # '-s', 'RELOOP=1',
  #'-s', '''EXPORTED_FUNCTIONS=["_main"]''',
  #'--closure', '1',
  '-s','NO_EXIT_RUNTIME=1',
  '--bind',
  #'-ID:\\OpenSource\\speex-1.2rc2\\include',
  #'--js-library', 'library.js'
  # '--js-transform', 'python appender.py'
]

source_files = [
  'h264bsd_transform.c',
  'h264bsd_util.c',
  'h264bsd_byte_stream.c',
  'h264bsd_seq_param_set.c',
  'h264bsd_pic_param_set.c',
  'h264bsd_slice_header.c',
  'h264bsd_slice_data.c',
  'h264bsd_macroblock_layer.c',
  'h264bsd_stream.c',
  'h264bsd_vlc.c',
  'h264bsd_cavlc.c',
  'h264bsd_nal_unit.c',
  'h264bsd_neighbour.c',
  'h264bsd_storage.c',
  'h264bsd_slice_group_map.c',
  'h264bsd_intra_prediction.c',
  'h264bsd_inter_prediction.c',
  'h264bsd_reconstruct.c',
  'h264bsd_dpb.c',
  'h264bsd_image.c',
  'h264bsd_deblocking.c',
  'h264bsd_conceal.c',
  'h264bsd_vui.c',
  'h264bsd_pic_order_cnt.c',
  'h264bsd_decoder.c',
  'H264SwDecApi.c',]
for file in source_files:
  target = file.replace('.c', '.o')
  print 'emcc %s -> %s' % (file, target)
  emscripten.Building.emcc(file, emcc_args + ['-DHAVE_CONFIG_H','-D__SSE__','-Isrc', '-Iinc'], os.path.join('obj', target))
  object_files = [os.path.join('obj', x.replace('.c', '.o')) for x in source_files];
print 'link -> %s' % 'avc.bc'
emscripten.Building.link(object_files, 'avc.bc')