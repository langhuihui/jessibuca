declare namespace Module {

    export class VideoDecoder {
      
      constructor(val: any);
      setCodec(vtype: number, extra: string): void;
      decode(input: string, isKeyFrame: string, timestamp: number): void;
      clear(): void;
    }

}