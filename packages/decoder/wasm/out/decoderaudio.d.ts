class AudioDecoder {
      
    constructor(val: any);
    setCodec(atype: number, extra: string): void;
    decode(input: string, timestamp: number): void;
    clear(): void;
};

export interface WASMModule extends EmscriptenModule {
    AudioDecoder: typeof AudioDecoder
}

export default function CreateModule(mod?: any): WASMModule;


