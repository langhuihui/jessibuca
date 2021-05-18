import os

os.system("emcc libavcodec.a libavutil.a libswresample.a -r -o ff.bc")