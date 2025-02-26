export const ERR = {
  NETWORK: 'network',
  NETWORK_TIMEOUT: 'network_timeout',
  NETWORK_FORBIDDEN: 'network_forbidden',
  OTHER: 'other',
  MANIFEST: 'manifest',
  HLS: 'hls',
  DEMUX: 'demux',
} as const;

type ErrorType = typeof ERR[keyof typeof ERR];

interface NetworkErrorPayload {
  url?: string;
  response?: Response;
  httpCode?: number;
}

interface NetworkErrorLike {
  isTimeout?: boolean;
  url?: string;
  response?: Response;
}

export class StreamingError extends Error {
  errorType: ErrorType;
  originError: Error | null;
  ext: unknown;
  errorMessage: string;

  constructor(
    type: ErrorType,
    subType: string | null,
    origin: Error | null,
    payload: unknown,
    msg?: string
  ) {
    super(msg || origin?.message);
    this.errorType = type === ERR.NETWORK_TIMEOUT ? ERR.NETWORK : type;
    this.originError = origin;
    this.ext = payload;
    this.errorMessage = this.message;
  }

  static create(
    type: ErrorType | StreamingError | Error,
    subType?: string | null,
    origin?: Error | null,
    payload?: unknown,
    msg?: string
  ): StreamingError {
    if (type instanceof StreamingError) {
      return type;
    } else if (type instanceof Error) {
      origin = type;
      type = ERR.OTHER;
    }

    return new StreamingError(type, subType || null, origin || null, payload, msg);
  }

  static network(error?: NetworkErrorLike): StreamingError {
    return new StreamingError(
      error?.isTimeout ? ERR.NETWORK_TIMEOUT : ERR.NETWORK,
      null,
      error instanceof Error ? error : null,
      {
        url: error?.url,
        response: error?.response,
        httpCode: error?.response?.status
      } as NetworkErrorPayload
    );
  }
} 