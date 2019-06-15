
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
emcc_args = [
    # '-m32',
    '-O3',
    '--memory-init-file', '0',
    '--llvm-opts', '3',
    # '--llvm-lto','1',
]

print 'building...'
object_files=[
     "bitstream.cc.o",
  "cabac.cc.o",
  "de265.cc.o",
  "deblock.cc.o",
  "decctx.cc.o",
  "nal-parser.cc.o",
  "dpb.cc.o",
  "image.cc.o",
  "intrapred.cc.o",
  "md5.cc.o",
  "nal.cc.o",
  "pps.cc.o",
  "transform.cc.o",
  "refpic.cc.o",
  "sao.cc.o",
  "scan.cc.o",
  "sei.cc.o",
  "slice.cc.o",
  "sps.cc.o",
  "util.cc.o",
  "vps.cc.o",
  "vui.cc.o",
  "motion.cc.o",
  "threads.cc.o",
  "visualize.cc.o",
  "fallback.cc.o",
  "fallback-motion.cc.o",
  "fallback-dct.cc.o",
  "quality.cc.o",
  "configparam.cc.o",
  "image-io.cc.o",
  "alloc_pool.cc.o",
  "en265.cc.o",
  "contextmodel.cc.o",
]
os.system('emcc ' +(' '.join(object_files+emcc_args)) + ' -o ../../../../obj/libde265.bc')

print 'done'
