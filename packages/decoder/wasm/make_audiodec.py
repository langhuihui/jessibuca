
#!/usr/bin/python

# import tools.shared as emscripten
import os
import sys
import getopt
from subprocess import Popen, PIPE, STDOUT
args = {'-o': './types/audiodec'}

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
    '-Isrc/common', '-Ithirdparty/audio/ffmpeg/include'
]+["-s "+k+"="+str(v) for k, v in sargs.items()]

print ('building...')

emcc_args = ['thirdparty/audio/ffmpeg/lib/libavcodec.a','thirdparty/audio/ffmpeg/lib/libavutil.a','thirdparty/audio/ffmpeg/lib/libswresample.a']+emcc_args

os.system('emcc ./src/audio/dec.cpp ./src/common/dec_audio_base.cpp ./src/common/dec_audio_ffmpeg.cpp ./src/common/dec_base_ffmpeg.cpp ' +
          (' '.join(emcc_args)) + ' -o '+args['-o']+'.js')

print ('done')
