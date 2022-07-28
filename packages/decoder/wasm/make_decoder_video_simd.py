
#!/usr/bin/python

# import tools.shared as emscripten
import os
import sys
import getopt
from subprocess import Popen, PIPE, STDOUT
args = {'-o': './out/decodervideo_simd'}

sargs = {
    'WASM': 1,
    'TOTAL_MEMORY': 67108864*8,
    'ASSERTIONS': 0,
    'ERROR_ON_UNDEFINED_SYMBOLS': 0,
    'DISABLE_EXCEPTION_CATCHING': 1,
    'INVOKE_RUN':0,
    'USE_PTHREADS':  0
}
emcc_args = [
    # '-m32',
    # '-fPIC',
    # '-Oz',
    '--memory-init-file', '0',
    # '--closure', '1',
    # '--llvm-lto','1',
    '--bind',
    '-I.', '-Ithirdparty/android', '-Ithirdparty/android/include', '-Ithirdparty/ffmpeg/include',
    '--post-js','./post.js',
    '-msimd128'
]+["-s "+k+"="+str(v) for k, v in sargs.items()]

print ('building...')

emcc_args = ['thirdparty/android/lib/libavcdec-simd.a', 'thirdparty/android/lib/libhevcdec-simd.a', 'thirdparty/ffmpeg/lib/libavcodec.a','thirdparty/ffmpeg/lib/libavutil.a','thirdparty/ffmpeg/lib/libswresample.a']+emcc_args

os.system('emcc ./src/decodervideo_simd/decodervideo.cpp ./thirdparty/android/log.c ./thirdparty/android/deocdervideo.cpp ./thirdparty/android/decoderavc.cpp ./thirdparty/android/decoderhevc.cpp ' +
          (' '.join(emcc_args)) + ' -o '+args['-o']+'.js')

print ('done')
