
class VideoDecoder {

    constructor(val: any);
    setCodec(vtype: number, extra: string): void;
    decode(input: string, isKeyFrame: string, timestamp: number): void;
    clear(): void;
    delete(): void;
};

interface VideoDecoderModuleWithBindings extends EmscriptenModule {

    VideoDecoder: typeof VideoDecoder
    
}
 
declare const VideoDecoderModule: EmscriptenModuleFactory<VideoDecoderModuleWithBindings>;
 
export default VideoDecoderModule;

