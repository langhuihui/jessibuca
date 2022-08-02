
#!/usr/bin/python

# import tools.shared as emscripten
import os
import sys
import getopt
from subprocess import Popen, PIPE, STDOUT
args = {'-o': './out/decoderaudio'}

sargs = {
    'WASM': 1,
    'TOTAL_MEMORY': 67108864*8,
    'ASSERTIONS': 0,
    'ERROR_ON_UNDEFINED_SYMBOLS': 0,
    'DISABLE_EXCEPTION_CATCHING': 1,
    'INVOKE_RUN':0,
    'USE_PTHREADS': 0,
    'MODULARIZE' : 1,
    'EXPORT_ES6' : 1
}
emcc_args = [
    # '-m32',
     '-O3',
    '--memory-init-file', '0',
    # '--closure', '1',
    # '--llvm-lto','1',
    '--bind',
    '-I.', '-Ithirdparty/ffmpeg/include',
    '--post-js','./post.js'
]+["-s "+k+"="+str(v) for k, v in sargs.items()]

print ('building...')

emcc_args = ['thirdparty/ffmpeg/lib/libavcodec.a','thirdparty/ffmpeg/lib/libavutil.a','thirdparty/ffmpeg/lib/libswresample.a']+emcc_args

os.system('emcc ./src/decoderaudio/decoderaudio.cpp ' +
          (' '.join(emcc_args)) + ' -o '+args['-o']+'.js')

print ('done')
