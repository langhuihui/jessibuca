
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
OBJ_DIR = "obj"
if not os.path.exists(OBJ_DIR):
    os.makedirs(OBJ_DIR)

emcc_args = [
    #'-m32',
    '--pre-js', os.path.join('../js', 'WebGLCanvas.js'),
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
    '-s', 'NO_EXIT_RUNTIME=1',
    '--bind',
    #'-I../speex-1.2rc2/include',
    '-I../Broadway',
    '-I../libid3tag',
    '-I.',
    '-DHAVE_CONFIG_H',
    '-DFPM_DEFAULT',
    '-DUSE_MP3', '-DDEBUG'
    #'--js-library', 'library.js'
    # '--js-transform', 'python appender.py'
]

source_files = []
FileNames = os.listdir(os.getcwd())
if (len(FileNames) > 0):
    for fn in FileNames:
        if(os.path.splitext(fn)[1] == '.c'):
            source_files.append(fn)
source_files.remove('minimad.c')
#build_files = ['bits.c','syntax.c','decoder.c']
build_files = source_files
for file in build_files:
    target = file.replace('.c', '.o')
    print 'emcc %s -> %s' % (file, target)
    os.system('emcc '+file+' ' + (' '.join(emcc_args)) + ' -o obj/'+target)
    # emscripten.Building.emcc(file, emcc_args, os.path.join('obj', target))

object_files = [os.path.join('obj', x.replace('.c', '.o'))
                for x in source_files]
print 'link -> %s' % 'mp3.bc'
os.system('emcc ' + (' '.join(object_files)) + ' -o ../obj/mp3.bc')
# emscripten.Building.link(object_files, '../obj/mp3.bc')
#print 'link -> %s' % 'MonaClient.bc'

# object_files = os.listdir('../obj')
# object_files.remove('MonaClient.bc')
# object_files = [os.path.join('../obj', x) for x in object_files]

# emscripten.Building.link(object_files, '../obj/MonaClient.bc')
# print 'emcc -> %s' % 'MonaClient.js'
# emscripten.Building.emcc('../obj/MonaClient.bc', emcc_args, '../js/MonaClient.js')
print 'done'
