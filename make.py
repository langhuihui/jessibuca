
#!/usr/bin/python

import os, sys, re, json, shutil
from subprocess import Popen, PIPE, STDOUT

exec(open(os.path.expanduser('~/.emscripten'), 'r').read())

sys.path.append(EMSCRIPTEN_ROOT)
import tools.shared as emscripten

emcc_args = [
'--pre-js',os.path.join('js','WebGLCanvas.js'),
  #'-m32',
  #'-O3',
 '--memory-init-file', '0',
 # '--llvm-opts', '3',
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
  '-Ispeex-1.2rc2/include',
  '-IBroadway','-I.',
  #'-I../libid3tag',
  '-DUSE_MP3',
  '-DUSE_H265',
  #'-DUSE_AAC',
  #'--js-library', 'library.js'
  # '--js-transform', 'python appender.py'
]

print 'build'
print 'emcc -> %s' % 'target.o'
emscripten.Building.emcc('MonaClient.cpp', emcc_args, 'obj/target.o')
print 'link -> %s' % 'target.bc'
object_files = os.listdir('obj')
if  'target.bc' in object_files:
  object_files.remove('target.bc')
object_files = [os.path.join('obj', x) for x in object_files]
emscripten.Building.link(object_files, 'obj/target.bc')
print 'emcc %s -> %s' % ('target.bc', 'MonaClient.js')
emscripten.Building.emcc('obj/target.bc', emcc_args, 'js/MonaClient.js')

print 'done'