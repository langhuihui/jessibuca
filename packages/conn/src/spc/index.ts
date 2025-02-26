import { Connection } from "../base";
import { ClientAbility, getClientAbility, getUplinkSSRCFromSDP, handleOfferSDP, ServerAbility } from "./sdp";
import transform from 'sdp-transform';
const enum VideoCodec {
  H264 = 'h264',
  VP8 = 'vp8'
}
export class SPConnection extends Connection {
  stat = {
    iceStartTime: 0,
    iceEndTime: 0,
    dtlsStartTime: 0,
    dtlsEndTime: 0,
    peerConnectionStartTime: 0,
    peerConnectionEndTime: 0
  };
  clientAbility: ClientAbility;
  private _serverAbility?: ServerAbility;
  private _peerConnection?: RTCPeerConnection
  private _parsedAnswer?: transform.SessionDescription
  async _connect() {
    this._peerConnection = new RTCPeerConnection({
      // @ts-ignore
      offerExtmapAllowMixed: true,
      bundlePolicy: 'max-bundle',
      rtcpMuxPolicy: 'require',
      // the two items below are non-standard & obsolete
      // tcpCandidatePolicy: 'disable',
      // IceTransportsType: 'nohost',
      ...this.options.rtcConfig
    });
    this._peerConnection.oniceconnectionstatechange = () => {
      if (!this._peerConnection) return;
      const iceState = this._peerConnection.iceConnectionState;
      if (iceState === 'checking' && this.stat.iceStartTime === 0) {
        this.stat.iceStartTime = Date.now();
      } else if (iceState === 'connected' && this.stat.iceEndTime === 0) {
        this.stat.iceEndTime = Date.now();
      } else if (iceState === 'failed') {
      }
    };
    this._peerConnection.onconnectionstatechange = () => this.onConnectionStateChange();
    this._peerConnection.addTransceiver('audio', {
      direction: 'sendonly'
    });
    this._peerConnection.addTransceiver('video', {
      direction: 'sendonly'
    });
    // this._peerConnection.addTransceiver('video', {
    //   direction: 'sendonly'
    // });
    // this._peerConnection.addTransceiver('video', {
    //   direction: 'sendonly'
    // });
    const offer = await this._peerConnection.createOffer();
    this.clientAbility = await getClientAbility(offer.sdp as string);
    await this.setOffer(offer);
    if (this.dtlsTransport) {
      this.dtlsTransport.onstatechange = () => {
        const { dtlsTransport } = this;
        if (!dtlsTransport) return;
        if (dtlsTransport.state === 'connecting' && this.stat.dtlsStartTime === 0) {
          this.stat.dtlsStartTime = Date.now();
        } else if (dtlsTransport.state === 'connected' && this.stat.dtlsEndTime === 0) {
          this.stat.dtlsEndTime = Date.now();
        }
      };
    }
    const res = await fetch(this.url, {
      method: "POST",
      body: offer.sdp,
      ...(this.options.requestInit || {}),
    });
    const answer = await res.text();
    await this._peerConnection!.setRemoteDescription({
      type: 'answer',
      sdp: answer
    });
    return;
  }
  setOffer(offer: RTCSessionDescriptionInit) {
    return this._peerConnection!.setLocalDescription({ type: 'offer', sdp: handleOfferSDP(offer.sdp!, this.clientAbility, this._serverAbility) });
  }
  onConnectionStateChange() {
    if (!this._peerConnection) return;
    const connectionState = this._peerConnection.connectionState;
    if (connectionState === 'connected') {
      this.stat.peerConnectionEndTime = Date.now();
    }
  }
  get dtlsTransport() {
    if (!this._peerConnection) return null;
    const senders = this._peerConnection.getSenders();
    if (senders.length === 0) return null;
    return senders[0].transport;
  }
  get videoCodec() {
    const codec = this._parsedAnswer?.media[1].rtp.find(item => [VideoCodec.H264, VideoCodec.VP8].includes(item.codec.toLowerCase() as VideoCodec));
    if (codec) return codec.codec.toLowerCase() as VideoCodec;
    return this._serverAbility?.useVp8 ? VideoCodec.VP8 : VideoCodec.H264;
  }
  get downlinkVideoCodec() {
    return this._serverAbility?.video.decoders.find(item => item.codec.toLowerCase() === VideoCodec.H264) ? VideoCodec.H264 : VideoCodec.VP8;
  }
  get is42001fSupported() {
    if (!this.clientAbility) return false;
    return !!this.clientAbility.video.codecs.find(item => item.fmtp.includes('42001f'));
  }
  get uplinkSSRC() {
    if (this._peerConnection && this._peerConnection.localDescription) {
      return getUplinkSSRCFromSDP(this._peerConnection.localDescription.sdp);
    }
    return {
      audioSsrc: 0,
      audioRtxSsrc: 0,
      bigVideoSsrc: 0,
      bigVideoRtxSsrc: 0,
      smallVideoSsrc: 0,
      smallVideoRtxSsrc: 0,
      auxVideoSsrc: 0,
      auxVideoRtxSsrc: 0
    };
  }
}