
#!/usr/bin/python

# import tools.shared as emscripten
import os
import sys
import re
import json
import shutil
from subprocess import Popen, PIPE, STDOUT

exec(open(os.path.expanduser('~/.emscripten'), 'r').read())

sys.path.append(EMSCRIPTEN_ROOT)
OBJ_DIR = "obj"
if not os.path.exists(OBJ_DIR):
    os.makedirs(OBJ_DIR)

emcc_args = [
    # '-m32',

    '-O3',
    '--memory-init-file', '0',
    '--llvm-opts', '3',
    '-I.',
    '-Icommon',
    '-Icommon/mips',
    '-Idecoder',
    '-Dasm=printf',
    '-DDEBUG_PRINT'
]


def compile(path):
    source_files = []

    FileNames = os.listdir(os.path.join(os.getcwd(), path))
    if (len(FileNames) > 0):
        for fn in FileNames:
            if(os.path.splitext(fn)[1] == '.c'):
                source_files.append(fn)

    build_files = source_files
    for file in build_files:
        target = file.replace('.c', '.o')
        if not os.path.exists(os.path.join('obj', target)):
            print 'emcc %s -> %s' % (file, target)
            
            os.system('emcc '+os.path.join(path, file)+' ' + (' '.join(emcc_args)) +
                    ' -o '+os.path.join('obj', target))

        #emscripten.Building.emcc(file, emcc_args, os.path.join('obj', target))

    return [os.path.join('obj', x.replace('.c', '.o'))
            for x in source_files]


object_files = compile('common')+compile('decoder') + \
    compile('decoder/mips')+compile('common/mips')

print 'link -> %s' % 'libhevc.bc'
os.system('emcc '+(' '.join(object_files))+' -o ../obj/libhevc.bc')
#emscripten.Building.link(object_files, '../obj/h265.bc')
# print 'link -> %s' % 'MonaClient.bc'

# object_files = os.listdir('../obj')
# object_files.remove('MonaClient.bc')
# object_files = [os.path.join('../obj', x) for x in object_files]

# emscripten.Building.link(object_files, '../obj/MonaClient.bc')
# print 'emcc -> %s' % 'MonaClient.js'
# emscripten.Building.emcc('../obj/MonaClient.bc', emcc_args, '../js/MonaClient.js')
print 'done'
