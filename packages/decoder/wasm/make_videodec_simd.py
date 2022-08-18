
#!/usr/bin/python

# import tools.shared as emscripten
import os
import sys
import getopt
from subprocess import Popen, PIPE, STDOUT
args = {'-o': './types/videodec_simd'}

sargs = {
    'WASM': 1,
    'TOTAL_MEMORY': 128*1024*1024,
    'ASSERTIONS': 0,
    'ERROR_ON_UNDEFINED_SYMBOLS': 0,
    'DISABLE_EXCEPTION_CATCHING': 1,
    'INVOKE_RUN':0,
    'USE_PTHREADS': 0,
    'MODULARIZE' : 1,
    'EXPORT_ES6' : 1
}
emcc_args = [
    '-O3',
    '--memory-init-file', '0',
    '-lembind',
    '-msimd128',
    '-Isrc/common', '-Ithirdparty/video'

]+["-s "+k+"="+str(v) for k, v in sargs.items()]

print ('building...')

emcc_args = ['thirdparty/video/libavc/lib/libavcdec-simd.a', 'thirdparty/video/libhevc/lib/libhevcdec-simd.a']+emcc_args

os.system('emcc ./src/video/dec_simd.cpp ./src/common/dec_video_base.cpp ./src/common/dec_avc_libavc.cpp ./src/common/dec_hevc_libhevc.cpp ' +
          (' '.join(emcc_args)) + ' -o '+args['-o']+'.js')

print ('done')
