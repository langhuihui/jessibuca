<template>
    <div class="player-notes" :class="'is-' + variant">
        <section v-if="variant === 'pro'" class="pn-card pn-notice">
            <div class="pn-notice-main">
                <p class="pn-kicker">试用</p>
                <ul>
                    <li>试用开发包约 1 小时断开，刷新页面可继续测试。</li>
                    <li>体验版 wasm 解码会在到期后暂停，更新官网最新包可继续。</li>
                    <li>离线包需挂在 Nginx 或本地 web server 上才能运行。</li>
                </ul>
            </div>
            <div class="pn-notice-side">
                <p>购买 Pro 可添加作者微信 <strong>bosswancheng</strong></p>
                <div class="pn-actions">
                    <a href="/pro.html">Pro 文档</a>
                    <a href="https://jessibuca.com/pro.zip">离线包</a>
                    <a href="http://jessibuca.monibuca.com/pro/index.html">Demo HTTP</a>
                    <a href="https://jessibuca.com/pro/index.html">Demo HTTPS</a>
                </div>
            </div>
        </section>

        <section v-else class="pn-card pn-notice">
            <div class="pn-notice-main">
                <p class="pn-kicker">开源版</p>
                <p class="pn-lead">浏览器里直接播 HTTP-FLV、WS-FLV，以及 M7S 私有格式（ws-raw）。HTTP 需设置 CORS；协议同时支持 https / wss。</p>
            </div>
            <div class="pn-notice-side">
                <div class="pn-actions">
                    <a href="/player-pro.html">体验 Pro</a>
                    <a href="/pro.html">Pro 差异</a>
                </div>
            </div>
        </section>

        <section class="pn-block">
            <div class="pn-head">
                <p class="pn-kicker">{{ variant === 'pro' ? '直播' : '协议' }}</p>
                <p class="pn-title">{{ variant === 'pro' ? '支持的直播地址' : '支持 3 种格式' }}</p>
                <p v-if="variant === 'pro'" class="pn-lead">示例 path 为 <code>live/test</code>。Chrome 访问 IP 可能报安全错误，建议用域名，并确认端口未被浏览器禁用。</p>
            </div>

            <template v-if="variant === 'pro'">
                <div v-for="group in liveGroups" :key="group.title" class="pn-group">
                    <p class="pn-subtitle">{{ group.title }}</p>
                    <div class="pn-list">
                        <article v-for="item in group.items" :key="item.name" class="pn-item">
                            <div class="pn-item-name">
                                <strong>{{ item.name }}</strong>
                                <span v-if="item.codec" class="pn-tag">{{ item.codec }}</span>
                            </div>
                            <code>{{ item.sample }}</code>
                            <p v-if="item.note" class="pn-item-note">
                                <a v-if="item.href" :href="item.href" :target="item.external ? '_blank' : null">{{ item.note }}</a>
                                <template v-else>{{ item.note }}</template>
                            </p>
                        </article>
                    </div>
                </div>
            </template>
            <div v-else class="pn-list">
                <article v-for="item in ossItems" :key="item.name" class="pn-item">
                    <div class="pn-item-name">
                        <strong>{{ item.name }}</strong>
                    </div>
                    <code>{{ item.sample }}</code>
                    <p v-if="item.note" class="pn-item-note">
                        <a v-if="item.href" :href="item.href" target="_blank">{{ item.note }}</a>
                        <template v-else>{{ item.note }}</template>
                    </p>
                </article>
            </div>
        </section>

        <section class="pn-block">
            <div class="pn-head">
                <p class="pn-kicker">能力</p>
                <p class="pn-title">{{ variant === 'pro' ? '编解码与扩展' : '编解码' }}</p>
            </div>
            <div class="pn-chips">
                <span v-for="item in capabilityChips" :key="item">{{ item }}</span>
            </div>
            <p v-if="variant === 'pro'" class="pn-links">
                <a href="/pro-demo.html#海康nvr点播文件-支持倍率播放，不支持seek">海康 NVR 点播（支持倍率，不支持 seek）</a>
                <a href="https://help.aliyun.com/zh/live/user-guide/web-rts-sdk-overview?spm=a2c4g.11186623.0.i1" target="_blank" rel="noopener noreferrer">阿里云 Web RTC</a>
                <a href="https://pili-player-demo.qiniu.com/" target="_blank" rel="noopener noreferrer">七牛云直播 Demo</a>
            </p>
        </section>

        <section v-if="variant === 'pro'" class="pn-block">
            <div class="pn-head">
                <p class="pn-kicker">点播</p>
                <p class="pn-title">两种点播格式</p>
                <p class="pn-lead">示例 path 为 <code>vod/test</code>。支持 H264 / H265，硬解码（MediaSource、WebCodecs）和软解码（WASM、WASM SIMD），以及国标 SM4 / XOR / m7s 加密文件。</p>
            </div>
            <div class="pn-list">
                <article v-for="item in vodItems" :key="item.name" class="pn-item">
                    <div class="pn-item-name">
                        <strong>{{ item.name }}</strong>
                        <span class="pn-tag">H264 / H265</span>
                    </div>
                    <code>{{ item.sample }}</code>
                </article>
            </div>
        </section>

        <section v-if="variant === 'oss'" class="pn-block">
            <div class="pn-head">
                <p class="pn-kicker">支持作者</p>
                <p class="pn-title">打赏</p>
            </div>
            <div class="pn-pay">
                <figure>
                    <figcaption>第一作者</figcaption>
                    <div class="pn-qr">
                        <img src="/wx.jpg" alt="第一作者微信">
                        <img src="/alipay.jpg" alt="第一作者支付宝">
                    </div>
                </figure>
                <figure>
                    <figcaption>V3 版本作者</figcaption>
                    <div class="pn-qr">
                        <img src="/wx-pay-wc.jpg" alt="V3 作者微信">
                        <img src="/alipay-wc.jpg" alt="V3 作者支付宝">
                    </div>
                </figure>
            </div>
        </section>

        <section class="pn-block">
            <div class="pn-head">
                <p class="pn-kicker">交流</p>
                <p class="pn-title">群与频道</p>
            </div>
            <div class="pn-qr pn-qr-community">
                <figure>
                    <img src="/qrcode.jpeg" alt="交流群二维码">
                    <figcaption>微信群</figcaption>
                </figure>
                <figure>
                    <img src="/qq-qrcode.jpg" alt="QQ 频道二维码">
                    <figcaption>QQ 频道</figcaption>
                </figure>
            </div>
        </section>
    </div>
</template>

<script>
export default {
    name: 'PlayerPageNotes',
    props: {
        variant: {
            type: String,
            default: 'pro'
        }
    },
    computed: {
        capabilityChips() {
            return this.variant === 'pro'
                ? [
                    'HTTP 需 CORS',
                    'https / wss',
                    'H264 / H265',
                    'MPEG-4',
                    'MSE / WebCodecs 硬解',
                    'HLS 软解 / 硬解',
                    'm7s WebRTC H264 / H265',
                    'ZLMediaKit / SRS / 其他 / 七牛 WebRTC H264',
                    '国标 SM4 / XOR / m7s 加密流',
                    '裸流 / fMP4 / MPEG-TS / MPEG-PS'
                ]
                : [
                    'HTTP 需 CORS',
                    'https / wss',
                    'H264 / H265 WASM 软解',
                    'WebCodecs / MSE 硬解（H264）'
                ]
        }
    },
    data() {
        return {
            ossItems: [
                {
                    name: 'ws-raw',
                    sample: 'ws://localhost/live/test',
                    note: '仅对接 monibuca 服务器',
                    href: 'https://monibuca.com'
                },
                {
                    name: 'ws-flv',
                    sample: 'ws://localhost/live/test.flv'
                },
                {
                    name: 'http-flv',
                    sample: 'http://localhost/live/test.flv'
                }
            ],
            vodItems: [
                { name: 'http(s)-mp4', sample: 'http(s)://host:port/hdl/vod/test.mp4' },
                { name: 'http(s)-hls', sample: 'http(s)://host:port/hdl/vod/test.m3u8' }
            ],
            liveGroups: [
                {
                    title: 'HTTP / WebSocket',
                    items: [
                        {
                            name: 'ws(s)-raw',
                            sample: 'ws(s)://host:port/jessica/live/test',
                            codec: 'm7s私有格式',
                            note: '仅支持 monibuca，其他服务器需额外对接',
                            href: 'https://monibuca.com',
                            external: true
                        },
                        {
                            name: 'ws(s)/http(s)-flv',
                            sample: 'ws(s)|http(s)://host:port/jessica/live/test.flv',
                            codec: 'flv'
                        },
                        {
                            name: 'HLS',
                            sample: 'http(s)://host:port/hls/live/test.m3u8',
                            codec: 'HLS'
                        },
                        {
                            name: 'http(s) / ws(s)-fmp4',
                            sample: 'http(s)|ws(s)://host:port/your-path/live/test.(f)mp4',
                            codec: 'fMP4'
                        },
                        {
                            name: 'http(s) / ws(s)-h264',
                            sample: 'http(s)|ws(s)://host:port/jessica/live/test.h264',
                            codec: 'h264裸流'
                        },
                        {
                            name: 'http(s) / ws(s)-h265',
                            sample: 'http(s)|ws(s)://host:port/jessica/live/test.h265',
                            codec: 'h265裸流'
                        },
                        {
                            name: 'http(s) / ws(s)-mpeg4',
                            sample: 'http(s)|ws(s)://host:port/your-path/live/test.mpeg4',
                            codec: 'MPEG-4'
                        },
                        {
                            name: 'http(s)-ts',
                            sample: 'http(s)://host:port/your-path/live/test.ts',
                            codec: 'MPEG-TS'
                        },
                        {
                            name: 'http(s)-ps',
                            sample: 'http(s)://host:port/your-path/live/test.ps',
                            codec: 'MPEG-PS'
                        }
                    ]
                },
                {
                    title: 'WebRTC / WebTransport',
                    items: [
                        {
                            name: 'WebTransport',
                            sample: 'wt://host:port/play/live/test',
                            note: '仅对接 monibuca',
                            href: 'https://monibuca.com',
                            external: true
                        },
                        {
                            name: 'WebRTC（m7s）',
                            sample: 'webrtc://host:port/webrtc/play/live/test',
                            codec: 'H264 / H265',
                            note: '仅 https 或 http://localhost'
                        },
                        {
                            name: 'WebRTC（ZLMediaKit）',
                            sample: 'webrtc://host:port/index/api/webrtc?app=live&stream=stream-name&type=play',
                            codec: 'H264 / H265',
                            note: '仅 https 或 http://localhost'
                        },
                        {
                            name: 'WebRTC（SRS）',
                            sample: 'webrtc://host:port/rtc/v1/play/live/test',
                            codec: 'H264 / H265',
                            note: '仅 https 或 http://localhost'
                        },
                        {
                            name: 'WebRTC（七牛）',
                            sample: 'webrtc://host:port/live/test',
                            codec: 'H264 / H265',
                            note: '仅 https 或 http://localhost'
                        },
                        {
                            name: 'WebRTC（其他）',
                            sample: 'webrtc://host:port/live/test',
                            codec: 'H264 / H265',
                            note: '仅 https 或 http://localhost'
                        },
                        {
                            name: 'ARTC（阿里云）',
                            sample: 'artc://host:port/xxxx',
                            codec: 'H264 / H265',
                            note: '按阿里云播放地址填写'
                        }
                    ]
                }
            ]
        }
    }
}
</script>

<style scoped>
.player-notes {
    display: flex;
    flex-direction: column;
    gap: 28px;
    margin: 8px auto 48px;
    max-width: 940px;
}

.pn-kicker {
    margin: 0 0 6px;
    color: var(--vp-c-brand-1);
    font-size: 12px;
    font-weight: 700;
    letter-spacing: 0.08em;
}

.pn-title {
    margin: 0 0 8px;
    color: var(--vp-c-text-1);
    font-size: 22px;
    font-weight: 700;
    line-height: 1.3;
}

.pn-lead,
.pn-notice-side p,
.pn-item-note {
    margin: 0;
    color: var(--vp-c-text-2);
    font-size: 14px;
    line-height: 1.7;
}

.pn-card,
.pn-item,
.pn-pay figure {
    border: 1px solid var(--vp-c-divider);
    border-radius: 14px;
    background: color-mix(in srgb, var(--vp-c-bg) 88%, transparent);
    backdrop-filter: blur(8px);
}

.pn-notice {
    display: grid;
    grid-template-columns: minmax(0, 1.4fr) minmax(0, 1fr);
    gap: 16px 24px;
    padding: 18px 20px;
}

.pn-notice ul {
    margin: 0;
    padding-left: 18px;
    color: var(--vp-c-text-2);
    font-size: 14px;
    line-height: 1.7;
}

.pn-notice-side {
    display: flex;
    flex-direction: column;
    justify-content: center;
    gap: 10px;
}

.pn-actions {
    display: flex;
    flex-wrap: wrap;
    gap: 8px;
}

.pn-actions a,
.pn-links a {
    display: inline-flex;
    align-items: center;
    height: 32px;
    padding: 0 12px;
    border: 1px solid var(--vp-c-divider);
    border-radius: 8px;
    background: var(--vp-c-bg);
    color: var(--vp-c-brand-1);
    font-size: 13px;
    font-weight: 600;
    text-decoration: none;
}

.pn-actions a:hover,
.pn-links a:hover {
    border-color: var(--vp-c-brand-1);
}

.pn-head {
    margin-bottom: 12px;
}

.pn-group + .pn-group {
    margin-top: 18px;
}

.pn-subtitle {
    margin: 0 0 8px;
    color: var(--vp-c-text-1);
    font-size: 15px;
    font-weight: 700;
}

.pn-list {
    display: grid;
    gap: 8px;
}

.pn-item {
    display: grid;
    gap: 4px;
    padding: 12px 14px;
}

.pn-item-name {
    display: flex;
    flex-wrap: wrap;
    align-items: center;
    gap: 8px;
}

.pn-item-name strong {
    color: var(--vp-c-text-1);
    font-size: 14px;
}

.pn-tag {
    display: inline-flex;
    align-items: center;
    height: 22px;
    padding: 0 8px;
    border-radius: 999px;
    background: var(--vp-c-brand-soft);
    color: var(--vp-c-brand-1);
    font-size: 12px;
    font-weight: 600;
}

.pn-item code {
    overflow-wrap: anywhere;
    color: var(--vp-c-text-1);
    font-size: 13px;
    background: transparent;
}

.pn-chips,
.pn-links {
    display: flex;
    flex-wrap: wrap;
    gap: 8px;
}

.pn-chips span {
    display: inline-flex;
    align-items: center;
    min-height: 28px;
    padding: 4px 10px;
    border-radius: 999px;
    background: var(--vp-c-bg-soft);
    color: var(--vp-c-text-2);
    font-size: 13px;
}

.pn-links {
    margin-top: 12px;
}

.pn-pay,
.pn-qr {
    display: flex;
    flex-wrap: wrap;
    gap: 16px;
}

.pn-pay figure,
.pn-qr-community figure {
    margin: 0;
    padding: 12px;
}

.pn-pay figcaption,
.pn-qr-community figcaption {
    margin: 0 0 8px;
    color: var(--vp-c-text-2);
    font-size: 13px;
    font-weight: 600;
    text-align: center;
}

.pn-qr img,
.pn-qr-community img {
    width: 180px;
    max-width: 100%;
    height: auto;
    border-radius: 8px;
}

.pn-pay .pn-qr img {
    width: 150px;
}

@media (max-width: 720px) {
    .pn-notice {
        grid-template-columns: 1fr;
        padding: 14px;
    }

    .pn-title {
        font-size: 20px;
    }

    .pn-qr img,
    .pn-qr-community img,
    .pn-pay .pn-qr img {
        width: min(46vw, 180px);
    }
}
</style>
