
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
    '-I..','-std=c++11'
]


def compile(path):
    source_files = [
        "bitstream.cc",
  "cabac.cc",
  "de265.cc",
  "deblock.cc",
  "decctx.cc",
  "nal-parser.cc",
  "dpb.cc",
  "image.cc",
  "intrapred.cc",
  "md5.cc",
  "nal.cc",
  "pps.cc",
  "transform.cc",
  "refpic.cc",
  "sao.cc",
  "scan.cc",
  "sei.cc",
  "slice.cc",
  "sps.cc",
  "util.cc",
  "vps.cc",
"vui.cc",
  "motion.cc" ,
  "threads.cc" ,
  "visualize.cc",
  "fallback.cc"  ,"fallback-motion.cc" ,
  "fallback-dct.cc",
  "quality.cc" ,
  "configparam.cc" ,
   "image-io.cc",
   "alloc_pool.cc",
  "contextmodel.cc"
    ]

    build_files = source_files
    for file in build_files:
        target = file.replace('.cc', '.o')
        if not os.path.exists(os.path.join('obj', target)):
            print 'emcc %s -> %s' % (file, target)
            
            os.system('emcc '+os.path.join(path, file)+' ' + (' '.join(emcc_args)) +
                    ' -o '+os.path.join('obj', target))

        #emscripten.Building.emcc(file, emcc_args, os.path.join('obj', target))

    return [os.path.join('obj', x.replace('.cc', '.o'))
            for x in source_files]


object_files = compile('.')

print 'link -> %s' % 'libde265.bc'
os.system('emcc '+(' '.join(object_files))+' -o ../../../obj/libde265.bc')

print 'done'
