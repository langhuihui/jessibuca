
#!/usr/bin/python

# import tools.shared as emscripten
import os
import sys
import getopt
from subprocess import Popen, PIPE, STDOUT
# exec(open(os.path.expanduser('~/.emscripten'), 'r').read())
# sys.path.append(EMSCRIPTEN_ROOT)
opts, args = getopt.getopt(sys.argv[1:], "o:", ["wasm"])
args = {'-o': 'public/ff'}
for op, value in opts:
    args[op] = value
args['-o'] = args['-o'] + ('_wasm' if '--wasm' in args else '')
sargs = {
    # 'USE_PTHREADS':  0 if '--cocos' in args else 1,
    'WASM': 1 if '--wasm' in args else 0,
    'TOTAL_MEMORY': 67108864,
    'ASSERTIONS': 0,
    'ERROR_ON_UNDEFINED_SYMBOLS': 0,
    'DISABLE_EXCEPTION_CATCHING': 1,
    # 'ALLOW_MEMORY_GROWTH':1,
    'ENVIRONMENT':'"worker"',
    'INVOKE_RUN':0,
    'USE_PTHREADS':  1
    # 'DEMANGLE_SUPPORT':1
}
emcc_args = [
    # '-m32',
    '-Oz',
    '--memory-init-file', '0',
    # '--closure', '1',
    # '--llvm-lto','1',
    '--bind',
    '-I.', '-Iobj/include'
]+["-s "+k+"="+str(v) for k, v in sargs.items()]

# if '--cocos' in args:
#     emcc_args.append('--post-js cocosCom.js')
print ('building...')

emcc_args = ['obj/lib/libavcodec.a','obj/lib/libavutil.a','obj/lib/libswresample.a']+emcc_args

os.system('emcc Jessibuca.cpp ' +
          (' '.join(emcc_args)) + ' -o '+args['-o']+'.js')

print ('done')
