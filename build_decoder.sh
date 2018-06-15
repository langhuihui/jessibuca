cd ffmpeg


emconfigure ./configure --extra-cflags="-c -O3" --cc="emcc" --cxx="em++" --ar="emar" --prefix=$(pwd)/../dist --enable-cross-compile --target-os=none --arch=x86_32 --cpu=generic \
    --enable-gpl --enable-version3 --disable-avdevice --disable-avformat --disable-swresample --disable-postproc --disable-avfilter \
    --disable-programs --disable-logging --disable-everything --enable-decoder=h264 --enable-small\
    --disable-asm --disable-doc --disable-devices --disable-network --disable-static --enable-shared \
    --disable-hwaccels --enable-parser=h264 --enable-parser=hevc --enable-parser=aac --disable-bsfs --disable-debug --disable-protocols --disable-indevs --disable-outdevs 
make