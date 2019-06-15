
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

# object_files = ['libavcodec.a', 'libavutil.a', 'libswscale.a']
# os.system('emcc ' + (' '.join(object_files)) +
#           ' -O3 -s TOTAL_MEMORY=134217728  -o ../../obj/ffmpeg.bc')
name = 'libavutil'

libavutil_files = list(
    filter(lambda file: file.endswith('.o'), os.listdir('./'+name)))

libavutil_files = [os.path.join('./'+name, x)
                   for x in libavutil_files]

name = 'libavcodec'

libavcodec_files = list(
    filter(lambda file: file.endswith('.o'), os.listdir('./'+name)))
libavcodec_files.remove('reverse.o')
libavcodec_files.remove('log2_tab.o')
libavcodec_files = [os.path.join('./'+name, x)
                    for x in libavcodec_files]
emscripten.Building.link(libavutil_files+libavcodec_files, '../../obj/ffmpeg.bc')
print 'done'
