declare module "types" {
    export interface ConnectionOptions {
        reconnectCount?: number;
        reconnectTimeout?: (reconnectionCount: number) => number;
        tls?: boolean;
        rtcConfig?: RTCConfiguration;
        requestInit?: RequestInit;
    }
    export const enum ConnectionState {
        CONNECTED = "connected",
        DISCONNECTED = "disconnected",
        RECONNECTED = "reconnected"
    }
    export const enum ConnectionEvent {
        Connecting = "connecting",
        Reconnecting = "reconnecting"
    }
}
declare module "base" {
    import { FSM } from "afsm";
    import { ConnectionOptions } from "types";
    import Oput from "oput";
    /**
     * 使用尾递归优化，计算斐波那契数
     * @export
     * @param {number} n
     * @param {number} [ac1=1]
     * @param {number} [ac2=1]
     * @return {number}
     */
    export function fibonacci(n: number, ac1?: number, ac2?: number): number;
    /**
     * 获取重连超时间隔
     * 根据斐波那契数来计算，最大超时间隔为 13s，最小间隔为 2s
     * @return {number}
     * @memberof SignalChannel
     */
    export function getReconnectionTimeout(reconnectionCount: number): number;
    export class TransmissionStatistics {
        total: number;
        _buffer: number;
        lastTime: number;
        _bps: number;
        add(size: number): void;
        get bps(): number;
    }
    export abstract class Connection extends FSM {
        url: string;
        options: ConnectionOptions;
        oput?: Oput;
        up: TransmissionStatistics;
        down: TransmissionStatistics;
        underlyingSink: UnderlyingSink<Uint8Array>;
        abortCtrl?: AbortController;
        constructor(url: string, options?: ConnectionOptions);
        read<T extends number | ArrayBufferView>(need: T): any;
        connect(): Promise<void>;
        abstract _connect(): Promise<ReadableStream<Uint8Array> | void>;
        _close(): void;
        _send(data: ArrayBufferLike | ArrayBufferView): void;
        reconnect(): Promise<void>;
        onConnected(readable: ReadableStream<Uint8Array> | void): Promise<void>;
        disconnect(reason: any): void;
        reconnectAfter(delay?: number, count?: number): void;
        close(): void;
        send(data: ArrayBufferLike | ArrayBufferView): void;
    }
}
declare module "ws" {
    import { Connection } from "base";
    export class WebSocketConnection extends Connection {
        ws?: WebSocket;
        _connect(): Promise<any>;
        _close(): void;
        _send(data: ArrayBufferLike | ArrayBufferView): void;
    }
}
declare module "wt" {
    import { Connection } from "base";
    export class WebTransportConnection extends Connection {
        wt?: WebTransport;
        writer?: WritableStreamDefaultWriter<any>;
        _connect(): Promise<ReadableStream<any>>;
        _close(): void;
        _send(data: ArrayBufferLike | ArrayBufferView): Promise<void>;
    }
}
declare module "http" {
    import { Connection } from "base";
    export class HttpConnection extends Connection {
        _connect(): Promise<ReadableStream<Uint8Array<ArrayBuffer>>>;
    }
}
declare module "webrtc" {
    import { EventEmitter } from "eventemitter3";
    import { Connection } from "base";
    export class DataChannelConnection extends Connection {
        dc: RTCDataChannel;
        constructor(dc: RTCDataChannel);
        _connect(): Promise<void | ReadableStream<Uint8Array>>;
        _close(): void;
        _send(data: ArrayBufferLike | ArrayBufferView | string | Blob): void;
    }
    export class WebRTCStream extends EventEmitter<{
        change: [MediaStream];
        close: [];
    }> {
        id: string;
        direction: RTCRtpTransceiverDirection;
        private _mediaStream?;
        get mediaStream(): MediaStream | undefined;
        set mediaStream(value: MediaStream | undefined);
        audioTransceiver: RTCRtpTransceiver;
        videoTransceiver: RTCRtpTransceiver;
        private _videoTrack?;
        private _audioTrack?;
        constructor(id: string, direction?: RTCRtpTransceiverDirection);
        set audioTrack(track: MediaStreamAudioTrack | undefined);
        get audioTrack(): MediaStreamAudioTrack | undefined;
        set videoTrack(track: MediaStreamVideoTrack | undefined);
        get videoTrack(): MediaStreamVideoTrack | undefined;
        close(): void;
    }
    export class WebRTCConnection extends Connection {
        webrtc: RTCPeerConnection;
        streams: any;
        videoTransceiver: RTCRtpTransceiver[];
        audioTransceiver: RTCRtpTransceiver[];
        _connect(): Promise<any>;
        addStream(stream: WebRTCStream): WebRTCStream;
        deleteStream(id: string): void;
        _close(): void;
    }
}
declare module "file" {
    import { Connection } from "base";
    export class FileConnection extends Connection {
        file: File;
        constructor(file: File);
        _connect(): Promise<ReadableStream<Uint8Array<ArrayBuffer>>>;
    }
}
declare module "index" {
    export { WebSocketConnection } from "ws";
    export { WebTransportConnection } from "wt";
    export { HttpConnection } from "http";
    export { WebRTCConnection, DataChannelConnection, WebRTCStream } from "webrtc";
    export { FileConnection } from "file";
    export const enum Protocol {
        WS = "ws://",
        WSS = "wss://",
        HTTP = "http://",
        HTTPS = "https://",
        WebTransport = "wt://",
        WebRTC = "webrtc://"
    }
    export function getURLType(url: string | File): "file" | "webrtc" | "ws" | "http" | "wt";
}
