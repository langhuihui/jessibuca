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
  '-Iinclude',
  #'--js-library', 'library.js'
  # '--js-transform', 'python appender.py'
]

libspeex_files = [
'bits.c','cb_search.c',
'exc_10_16_table.c','exc_10_32_table.c',
'exc_20_32_table.c','exc_5_256_table.c',
'exc_5_64_table.c','exc_8_128_table.c',
'filters.c',
'gain_table.c','gain_table_lbr.c',
'hexc_10_32_table.c','hexc_table.c',
'high_lsp_tables.c','kiss_fft.c',
'kiss_fftr.c','lpc.c','lsp.c','lsp_tables_nb.c',
'ltp.c','modes.c','modes_wb.c',
'nb_celp.c','quant_lsp.c',
'sb_celp.c','smallft.c',
'speex.c','speex_callbacks.c','speex_header.c',
'stereo.c',
'vbr.c','vq.c','window.c'
]
for file in libspeex_files:
  target = file.replace('.c', '.o')
  print 'emcc %s -> %s' % (file, target)
  emscripten.Building.emcc('libspeex\\' + file, emcc_args + ['-DHAVE_CONFIG_H','-D__SSE__','-Isrc', '-Iinc','-I.'], os.path.join('obj', target))
libspeex_files = [os.path.join('obj', x.replace('.c', '.o')) for x in libspeex_files];
print 'link -> %s' % 'libspeex.bc'
emscripten.Building.link(libspeex_files, 'libspeex.bc')