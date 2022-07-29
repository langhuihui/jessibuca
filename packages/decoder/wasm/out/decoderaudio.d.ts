declare namespace Module {

    export class AudioDecoder {
      
      constructor(val: any);
      setCodec(atype: number, extra: string): void;
      decode(input: string, timestamp: number): void;
      clear(): void;
    }

}