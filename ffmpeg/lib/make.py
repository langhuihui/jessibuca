
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

print 'build'

object_files = ['libavcodec.a', 'libavutil.a', 'libswscale.a']
os.system('emcc ' + (' '.join(object_files)) +
          ' -O3 -s TOTAL_MEMORY=134217728  -o ../../obj/ffmpeg.bc')

print 'done'
