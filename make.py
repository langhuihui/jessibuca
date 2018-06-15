
#!/usr/bin/python

import os
import sys
import re
import json
import shutil
from subprocess import Popen, PIPE, STDOUT

exec(open(os.path.expanduser('~/.emscripten'), 'r').read())

sys.path.append(EMSCRIPTEN_ROOT)
import tools.shared as emscripten

emcc_args = [
    '--pre-js', os.path.join('js', 'WebGLCanvas.js'),
    #'-m32',
    '-O3',
    '--memory-init-file', '0',
    '--llvm-opts', '3',
    # '-s', 'WASM=1',
    #'-s', 'CORRECT_SIGNS=1',
    #'-s', 'CORRECT_OVERFLOWS=1',
    '-s', 'TOTAL_MEMORY=67108864',
    #'-s', 'FAST_MEMORY=' + str(12*1024*1024),
    #'-s', 'INVOKE_RUN=0',
    '-s', 'ASSERTIONS=1',
    #'-s DEMANGLE_SUPPORT=1',
    # '-s', 'RELOOP=1',
    #'-s', '''EXPORTED_FUNCTIONS=["_main"]''',
    #'--closure', '1',
    '-s', 'NO_EXIT_RUNTIME=1',
    '--bind',
    #'-Ispeex-1.2rc2/include',
    '-IBroadway', '-I.',
    #'-I../libid3tag',
    '-Iffmpeg/include',
    '-I../ffmpeg',
    # '-DUSE_MP3',
    #'-DUSE_LIBDE265',
    '-DUSE_AAC',
    '-DUSE_FFMPEG',
    '--js-library',  os.path.join('js', 'MonaMain.js')
    # '--js-library',  os.path.join('js', 'FlvMain.js')
]

print 'build'

object_files = ['aac.bc']
object_files = [os.path.join('obj', x) for x in object_files]
name = 'libavutil'

libavutil_files = list(
    filter(lambda file: file.endswith('.o'), os.listdir('../ffmpeg/'+name)))

libavutil_files = [os.path.join('../ffmpeg/'+name, x) for x in libavutil_files]

name = 'libavcodec'

libavcodec_files = list(
    filter(lambda file: file.endswith('.o'), os.listdir('../ffmpeg/'+name)))
libavcodec_files.remove('reverse.o')
libavcodec_files.remove('log2_tab.o')
libavcodec_files = [os.path.join('../ffmpeg/'+name, x)
                    for x in libavcodec_files]

# os.system('emcc FlvClient.cpp ' +
#           (' '.join(libavutil_files+libavcodec_files+object_files+emcc_args)) + ' -o js/FlvClient.js')

os.system('emcc MonaClient.cpp ' +
          (' '.join(libavutil_files+libavcodec_files+object_files+emcc_args)) + ' -o js/MonaClient.js')
print 'done'
