
#!/usr/bin/python

import os, sys, re, json, shutil
from subprocess import Popen, PIPE, STDOUT

exec(open(os.path.expanduser('~/.emscripten'), 'r').read())

sys.path.append(EMSCRIPTEN_ROOT)
import tools.shared as emscripten
OBJ_DIR = "obj"
if not os.path.exists(OBJ_DIR):
  os.makedirs(OBJ_DIR)

emcc_args = [
  #'-m32',
  '--pre-js',os.path.join('../js','WebGLCanvas.js'),
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
    '-I../speex-1.2rc2/include',
  '-I../Broadway',
  '-I.','-Imsvc++',
  '-DHAVE_CONFIG_H',
  '-DFPM_DEFAULT',
  '-DUSE_MP3'
  #'--js-library', 'library.js'
  # '--js-transform', 'python appender.py'
]

source_files = ['tag.c','frame.c','parse.c']  
''''
FileNames=os.listdir(os.getcwd())  
if (len(FileNames)>0):  
    for fn in FileNames:  
        if(os.path.splitext(fn)[1]=='.c'):
            source_files.append(fn)  
            '''
#source_files.remove('file.c')
#build_files = ['bits.c','syntax.c','decoder.c']
build_files =source_files
for file in build_files:
  target = file.replace('.c', '.o')
  print 'emcc %s -> %s' % (file, target)
  emscripten.Building.emcc(file, emcc_args, os.path.join('obj', target))

object_files = [os.path.join('obj', x.replace('.c', '.o')) for x in source_files]
print 'link -> %s' % 'id3tag.bc'
emscripten.Building.link(object_files, '../obj/id3tag.bc')
print 'link -> %s' % 'MonaClient.bc'
emscripten.Building.link(['../obj/avc.bc','../obj/id3tag.bc','../obj/mp3.bc','../obj/libspeex.bc','../obj/MonaClient.o'], '../obj/MonaClient.bc')
print 'emcc -> %s' % 'MonaClient.js'
emscripten.Building.emcc('../obj/MonaClient.bc', emcc_args, '../js/MonaClient.js')
print 'done'