
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
opts, args = getopt.getopt(sys.argv[1:], "v:a:o:", ["wasm"])
args = {}
for op, value in opts:
    args[op] = value
video_codec = '-DUSE_'+(args['-v']).upper() if '-v' in args else ''
audio_codec = '-DUSE_'+(args['-a']).upper() if '-a' in args else '-DUSE_MP3'
emcc_args = [
    # '-m32',
    '-O3',
    '--memory-init-file', '0',
    '--llvm-opts', '3',
    '-s', 'WASM='+('1' if '--wasm' in args else '0'),
    #'-s', 'CORRECT_SIGNS=1',
    #'-s', 'CORRECT_OVERFLOWS=1',
    '-s', 'TOTAL_MEMORY=67108864',
    #'-s', 'FAST_MEMORY=' + str(12*1024*1024),
    #'-s', 'INVOKE_RUN=0',
    '-s', 'ASSERTIONS=1',
    #'-s DEMANGLE_SUPPORT=1',
    # '-s', 'RELOOP=1',
    #'-s', '''EXPORTED_FUNCTIONS=["_main"]''',
    # '--closure', '1',
    # '--llvm-lto','1',
    '-s', 'NO_EXIT_RUNTIME=1',
    '--bind',
    '-I.', '-IBroadway',
    video_codec, audio_codec,
    # '-DUSE_LIBDE265',
    # '-DUSE_AAC',
    # '-DUSE_FFMPEG',
    '--js-library', 'H5LiveClient.js', '-s ERROR_ON_UNDEFINED_SYMBOLS=0'
]

print 'building...'
output_file = args['-o'] if '-o' in args else 'public/H5LiveClient.js'

object_files = []
if video_codec == '-DUSE_LIBHEVC':
    emcc_args.append('-Ilibhevc/decoder')
    emcc_args.append('-Ilibhevc/common')
    object_files.append('libhevc.bc')
elif video_codec == '-DUSE_LIBDE265':
    emcc_args.append('-Ilibde265')
    object_files.append('libde265.bc')
elif video_codec == '-DUSE_FFMPEG':
    emcc_args.append('-Iffmpeg/include')
    object_files.append('ffmpeg.bc')
else:
    object_files.append('avc.bc')
if audio_codec == '-DUSE_AAC':
    object_files.append('aac.bc')
elif audio_codec == '-DUSE_SPEEX':
    emcc_args.append('-Ispeex-1.2rc2/include')
    object_files.append('libspeex.bc')
else:
    object_files.append('mp3.bc')
print object_files
# emscripten.Building.emcc('H5LiveClient.cpp', [os.path.join(
#     'obj', x) for x in object_files]+emcc_args, output_file)
object_files = [os.path.join('obj', x) for x in object_files]
os.system('emcc H5LiveClient.cpp ' +
          (' '.join(object_files+emcc_args)) + ' -o '+output_file)

print 'done'
