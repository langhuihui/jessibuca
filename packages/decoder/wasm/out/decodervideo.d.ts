
class VideoDecoder {

    constructor(val: any);
    setCodec(vtype: number, extra: string): void;
    decode(input: string, isKeyFrame: string, timestamp: number): void;
    clear(): void;
    delete(): void;
};

export interface WASMModule extends EmscriptenModule {
    VideoDecoder: typeof VideoDecoder
}

export default function CreateModule(mod?: any): WASMModule;


