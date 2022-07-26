interface WebTransportDatagramDuplexStream {
    readonly readable: ReadableStream;
    readonly writable: WritableStream;
    readonly maxDatagramSize: number;
    incomingMaxAge: number;
    outgoingMaxAge: number;
    incomingHighWaterMark: number;
    outgoingHighWaterMark: number;
}
interface WebTransport {
    new (url: string, options?: WebTransportOptions);
    getStats(): Promise<WebTransportStats>;
    readonly ready: Promise<undefined>;
    readonly closed: Promise<WebTransportCloseInfo>;
    close(closeInfo?: WebTransportCloseInfo): undefined;
    readonly datagrams: WebTransportDatagramDuplexStream;
    createBidirectionalStream(): Promise<WebTransportBidirectionalStream>;
    readonly incomingBidirectionalStreams: ReadableStream;
    createUnidirectionalStream(): Promise<WritableStream>;
    readonly incomingUnidirectionalStreams: ReadableStream;
}
declare var WebTransport: {
  prototype: WebTransport;
  new (url: string, options?: WebTransportOptions): WebTransport;
};
interface WebTransportHash {
    algorithm?: string;
    value?: BufferSource;
}
interface WebTransportOptions {
    allowPooling?: boolean;
    serverCertificateHashes?: Array<WebTransportHash>;
}
interface WebTransportCloseInfo {
    closeCode?: number;
    reason?: string;
}
interface WebTransportStats {
    timestamp?: DOMHighResTimeStamp;
    bytesSent?: number;
    packetsSent?: number;
    numOutgoingStreamsCreated?: number;
    numIncomingStreamsCreated?: number;
    bytesReceived?: number;
    packetsReceived?: number;
    minRtt?: DOMHighResTimeStamp;
    numReceivedDatagramsDropped?: number;
}
interface WebTransportBidirectionalStream {
    readonly readable: ReadableStream;
    readonly writable: WritableStream;
}
interface WebTransportError extends DOMException {
    new (init?: WebTransportErrorInit);
    readonly source: WebTransportErrorSource;
    readonly streamErrorCode: number;
}
interface WebTransportErrorInit {
    streamErrorCode?: number;
    message?: string;
}
type WebTransportErrorSource = "stream" | "session";