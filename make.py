
#!/usr/bin/python

# import tools.shared as emscripten
import os
import sys
import re
import json
import shutil
import getopt
from subprocess import Popen, PIPE, STDOUT
exec(open(os.path.expanduser('~/.emscripten'), 'r').read())
# sys.path.append(EMSCRIPTEN_ROOT)
opts, args = getopt.getopt(sys.argv[1:], "v:a:o:", ["wasm", 'cocos'])
args = {'-o': 'public/Jessibuca.js'}
for op, value in opts:
    if value == 'ff':
        value = 'ffmpeg'
    args[op] = value

video_codec = '-DUSE_'+(args['-v']).upper() if '-v' in args else ''
audio_codec = '-DUSE_'+(args['-a']).upper() if '-a' in args else ''
sargs = {
    # 'USE_PTHREADS':  0 if '--cocos' in args else 1,
    'WASM': 1 if '--wasm' in args else 0,
    'TOTAL_MEMORY': 67108864,
    'ASSERTIONS': 0,
    'NO_EXIT_RUNTIME': 1,
    'ERROR_ON_UNDEFINED_SYMBOLS': 0,
    'DISABLE_EXCEPTION_CATCHING': 1,
    'ALLOW_MEMORY_GROWTH':1
    # 'INVOKE_RUN':1
    # 'DEMANGLE_SUPPORT':1
}
emcc_args = [
    # '-m32',
    '-O3',
    '--memory-init-file', '0',
    # '--closure', '1',
    # '--llvm-lto','1',
    '--bind',
    '-I.', '-Ithirdparty/Broadway',
    '-Ithirdparty',
    video_codec, audio_codec,
    '--js-library', 'cocos.js' if '--cocos' in args else 'Jessibuca.js',
]+["-s "+k+"="+str(v) for k, v in sargs.items()]

# if '--cocos' in args:
#     emcc_args.append('--post-js cocosCom.js')
print ('building...')

object_files = []
if video_codec == '-DUSE_LIBHEVC':
    emcc_args.append('-Dasm=printf')
    emcc_args.append('-Ithirdparty/libhevc/decoder')
    emcc_args.append('-Ithirdparty/libhevc/common')
    object_files.append('obj/libhevc.bc')
elif video_codec == '-DUSE_LIBDE265':
    emcc_args.append('-Ithirdparty/libde265')
    object_files.append('libde265.bc')
elif video_codec == '-DUSE_FFMPEG':
    emcc_args.append('-Ithirdparty/ffmpeg/include')
    object_files.append('obj/ffmpeg.bc')
    
else:
    object_files.append('avc.bc')
if audio_codec == '-DUSE_AAC':
    emcc_args.append('-Ithirdparty/aacDecoder/include')
    object_files.append('obj/aac.bc')
elif audio_codec == '-DUSE_SPEEX':
    emcc_args.append('-Ithirdparty/speex-1.2rc2/include')
    object_files.append('obj/libspeex.bc')
elif audio_codec == '-DUSE_MP3':
    object_files.append('obj/mp3.bc')
print (object_files)
# emscripten.Building.emcc('Jessibuca.cpp', [os.path.join(
#     'obj', x) for x in object_files]+emcc_args, output_file)
emcc_args = object_files+emcc_args
os.system('emcc Jessibuca.cpp ' +
          (' '.join(emcc_args)) + ' -o '+args['-o'])

print ('done')
