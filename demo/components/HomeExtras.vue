<template>
    <section v-if="part === 'stats'" class="home-stats" aria-label="开源版能力">
        <div v-for="item in stats" :key="item.label" class="home-stat">
            <strong>{{ item.value }}</strong>
            <span>{{ item.label }}</span>
        </div>
    </section>

    <div v-else class="home-extras">
        <section class="home-block">
            <p class="home-kicker">开源版</p>
            <h2>浏览器里直接播</h2>
            <p class="home-lead">开源版可播 HTTP-FLV、WS-FLV，以及 M7S 私有格式（ws-raw）。HLS、裸流、WebRTC 等在 Pro 区单独标明。</p>
            <div class="home-tags" aria-label="开源版协议">
                <span v-for="item in ossProtocols" :key="item">{{ item }}</span>
            </div>
        </section>

        <section class="home-block home-compare">
            <p class="home-kicker">版本差异</p>
            <h2>开源版能做什么，Pro 补什么</h2>
            <p class="home-lead">对照 <a href="/pro">Pro 文档差异表</a>。首页不再把两边能力写进同一句话。</p>
            <div class="home-compare-grid">
                <article class="home-compare-card">
                    <p class="home-compare-label">开源版</p>
                    <ul>
                        <li v-for="item in ossDiffs" :key="item">{{ item }}</li>
                    </ul>
                </article>
                <article class="home-compare-card home-compare-card-pro">
                    <p class="home-compare-label">
                        <span class="pro-badge">Pro</span>
                        仅 Pro 支持
                    </p>
                    <ul>
                        <li v-for="item in proDiffs" :key="item">{{ item }}</li>
                    </ul>
                </article>
            </div>
        </section>

        <section class="home-block home-pro">
            <div class="home-pro-inner">
                <p class="home-kicker">
                    <span class="pro-badge">Pro</span>
                    Jessibuca Pro
                </p>
                <h2>面向业务的直播与点播能力</h2>
                <p class="home-lead">以下能力均不在开源版内。硬解码、国标回放流、加密流、多屏与点播，在开源版之上补齐生产场景。</p>
                <div class="home-pro-grid">
                    <article v-for="item in proItems" :key="item.title">
                        <div class="home-card-head">
                            <h3>{{ item.title }}</h3>
                            <span class="pro-badge">Pro</span>
                        </div>
                        <p>{{ item.details }}</p>
                    </article>
                </div>
                <div class="home-protocol-pro">
                    <p class="home-protocol-pro-label">Pro 协议与封装</p>
                    <div class="home-tags home-tags-pro">
                        <span v-for="item in proProtocols" :key="item">{{ item }}</span>
                    </div>
                </div>
                <div class="home-pro-actions">
                    <a class="home-btn home-btn-brand" href="/player-pro">体验 Pro 播放器</a>
                    <a class="home-btn" href="/pro">查看完整差异表</a>
                </div>
            </div>
        </section>
    </div>
</template>

<script>
export default {
    name: 'HomeExtras',
    props: {
        part: {
            type: String,
            default: 'body'
        }
    },
    data() {
        return {
            stats: [
                { value: '<1s', label: '开源版直播延迟' },
                { value: '720P', label: 'WASM 软解建议上限' },
                { value: 'H.265', label: '开源版 WASM 软解' },
                { value: '3 协议', label: 'M7S 私有格式 / HTTP-FLV / WS-FLV' }
            ],
            ossProtocols: [
                'ws-raw（M7S 私有格式）',
                'HTTP-FLV',
                'WS-FLV'
            ],
            proProtocols: [
                'HLS 直播',
                'fMP4',
                'H.264 / H.265 裸流',
                'MPEG-TS / PS',
                'WebTransport',
                'WebRTC',
                '阿里云 RTC',
                '加密流'
            ],
            ossDiffs: [
                'H.264 / H.265 WASM 软解（720P）',
                'H.264 MSE / WebCodecs 硬解',
                'http-flv、websocket-flv、ws-raw（M7S 私有格式）',
                'AAC / G.711 音频，WebWorker 多实例',
                'WebM 录制，基础控制栏与快捷键'
            ],
            proDiffs: [
                'H.265 MSE / WebCodecs 硬解，SIMD / 多线程 1080P+',
                'HLS、fMP4、裸流、TS/PS、WebRTC、WebTransport',
                '国标 SM4 / XOR / M7S / HLS AES 加密流',
                'GB28181、GA/T 1078 回放与点播 MP4 / HLS',
                'MP4 / FLV 录制，对讲、PTZ、不规则多屏'
            ],
            proItems: [
                {
                    title: 'H.265 硬解码',
                    details: 'MSE / WebCodecs 硬解 H.265；WASM SIMD 与多线程可到 1080P 及以上。开源版 H.265 仅 WASM 软解。'
                },
                {
                    title: 'WebRTC 与更多协议',
                    details: 'WebRTC（含阿里云 / 七牛 / 大疆等）、WebTransport、HLS、fMP4、裸流、MPEG-TS / PS。开源版不含这些协议。'
                },
                {
                    title: '国标回放流',
                    details: 'GB28181、GA/T 1078 回放，支持暂停、倍速与 seek。开源版无录像回放能力。'
                },
                {
                    title: '加密流',
                    details: '国标 SM4、XOR、m7s-crypto、HLS AES-128-CBC 解密播放。开源版不支持加密流。'
                },
                {
                    title: '点播 MP4 / HLS',
                    details: '硬解 + 软解播放点播文件，覆盖 PS / TS / FLV / fMP4 等封装。开源版只做直播拉流。'
                },
                {
                    title: '多屏 · 对讲 · 录制',
                    details: '不规则多屏、语音通讯、PTZ，以及 MPEG-4 MP4 / FLV 录制。开源版录制为 WebM。'
                }
            ]
        }
    }
}
</script>

<style scoped>
.home-stats {
    display: grid;
    grid-template-columns: repeat(2, minmax(0, 1fr));
    gap: 12px;
    max-width: 1152px;
    margin: 0 auto 8px;
    padding: 0 24px 24px;
}

.home-stat {
    display: flex;
    flex-direction: column;
    gap: 4px;
    padding: 18px 20px;
    border: 1px solid var(--vp-c-divider);
    border-radius: 16px;
    background: var(--vp-c-bg-elv);
    box-shadow: 0 10px 30px rgba(91, 33, 182, 0.04);
}

.home-stat strong {
    font-size: 22px;
    font-weight: 700;
    letter-spacing: -0.03em;
    color: var(--vp-c-brand-1);
    line-height: 1.2;
}

.home-stat span {
    color: var(--vp-c-text-2);
    font-size: 13px;
    font-weight: 500;
    line-height: 1.4;
}

.home-extras {
    margin-top: 24px;
}

.home-block {
    max-width: 1152px;
    margin: 0 auto;
    padding: 48px 24px 8px;
}

.home-kicker {
    display: flex;
    align-items: center;
    gap: 8px;
    margin: 0 0 8px;
    font-size: 12px;
    font-weight: 700;
    letter-spacing: 0.16em;
    text-transform: uppercase;
    color: var(--vp-c-brand-1);
}

.home-block h2 {
    margin: 0 0 12px;
    border: 0;
    padding: 0;
    font-size: 28px;
    font-weight: 700;
    line-height: 1.3;
    letter-spacing: -0.02em;
    color: var(--vp-c-text-1);
}

.home-lead {
    margin: 0 0 24px;
    max-width: 42em;
    color: var(--vp-c-text-2);
    font-size: 16px;
    line-height: 1.7;
}

.home-lead a {
    color: var(--vp-c-brand-1);
    font-weight: 600;
    text-decoration: none;
}

.home-lead a:hover {
    text-decoration: underline;
}

.home-tags {
    display: flex;
    flex-wrap: wrap;
    gap: 10px;
}

.home-tags span {
    display: inline-flex;
    align-items: center;
    padding: 7px 14px;
    border: 1px solid var(--vp-c-divider);
    border-radius: 999px;
    background: var(--vp-c-bg-soft);
    color: var(--vp-c-text-1);
    font-size: 13px;
    font-weight: 600;
}

.home-tags-pro span {
    border-color: rgba(180, 83, 9, 0.22);
    background: #fff7ed;
    color: #9a3412;
}

.pro-badge {
    display: inline-flex;
    align-items: center;
    justify-content: center;
    height: 20px;
    padding: 0 7px;
    border-radius: 999px;
    background: linear-gradient(135deg, #ea580c 0%, #c2410c 100%);
    color: #fff;
    font-size: 11px;
    font-weight: 800;
    letter-spacing: 0.06em;
    text-transform: uppercase;
    line-height: 1;
}

.home-compare-grid {
    display: grid;
    grid-template-columns: 1fr;
    gap: 16px;
}

.home-compare-card {
    padding: 22px 24px 18px;
    border: 1px solid var(--vp-c-divider);
    border-radius: 16px;
    background: var(--vp-c-bg-elv);
}

.home-compare-card-pro {
    border-color: rgba(234, 88, 12, 0.22);
    background: linear-gradient(180deg, #fff7ed 0%, #fff 48%);
}

.home-compare-label {
    display: flex;
    align-items: center;
    gap: 8px;
    margin: 0 0 12px;
    font-size: 13px;
    font-weight: 700;
    letter-spacing: 0.08em;
    text-transform: uppercase;
    color: var(--vp-c-text-2);
}

.home-compare-card ul {
    margin: 0;
    padding: 0 0 0 18px;
    list-style: disc;
    color: var(--vp-c-text-1);
    font-size: 14px;
    line-height: 1.7;
}

.home-compare-card li + li {
    margin-top: 6px;
}

.home-pro {
    max-width: none;
    margin-top: 40px;
    padding: 56px 0 8px;
    background:
        linear-gradient(180deg, rgba(255, 247, 237, 0.92) 0%, rgba(255, 255, 255, 0) 100%);
    border-top: 1px solid rgba(234, 88, 12, 0.12);
}

.home-pro-inner {
    max-width: 1152px;
    margin: 0 auto;
    padding: 0 24px;
}

.home-pro-grid {
    display: grid;
    grid-template-columns: 1fr;
    gap: 16px;
}

.home-pro-grid article {
    padding: 22px 22px 20px;
    border: 1px solid rgba(234, 88, 12, 0.16);
    border-radius: 16px;
    background: var(--vp-c-bg-elv);
    box-shadow: 0 8px 24px rgba(15, 23, 42, 0.04);
    transition: transform 0.2s ease, box-shadow 0.2s ease, border-color 0.2s ease;
}

.home-pro-grid article:hover {
    transform: translateY(-3px);
    border-color: #ea580c;
    box-shadow: 0 16px 36px rgba(194, 65, 12, 0.1);
}

.home-card-head {
    display: flex;
    align-items: flex-start;
    justify-content: space-between;
    gap: 12px;
    margin-bottom: 8px;
}

.home-pro-grid h3 {
    margin: 0;
    font-size: 16px;
    font-weight: 600;
    line-height: 1.4;
}

.home-pro-grid p {
    margin: 0;
    color: var(--vp-c-text-2);
    font-size: 14px;
    line-height: 1.65;
}

.home-protocol-pro {
    margin-top: 28px;
}

.home-protocol-pro-label {
    margin: 0 0 12px;
    font-size: 13px;
    font-weight: 700;
    color: #9a3412;
}

.home-pro-actions {
    display: flex;
    flex-wrap: wrap;
    gap: 12px;
    margin-top: 28px;
}

.home-btn {
    display: inline-flex;
    align-items: center;
    justify-content: center;
    min-width: 148px;
    height: 40px;
    padding: 0 18px;
    border: 1px solid var(--vp-c-divider);
    border-radius: 22px;
    background: var(--vp-c-bg-elv);
    color: var(--vp-c-text-1);
    font-size: 14px;
    font-weight: 600;
    text-decoration: none;
}

.home-btn-brand {
    border-color: transparent;
    background: #c2410c;
    color: #fff;
}

.home-btn:hover {
    border-color: #c2410c;
    color: #c2410c;
}

.home-btn-brand:hover {
    background: #9a3412;
    color: #fff;
}

:global(html.dark) .home-tags-pro span {
    border-color: rgba(251, 146, 60, 0.28);
    background: rgba(154, 52, 18, 0.28);
    color: #fdba74;
}

:global(html.dark) .home-compare-card-pro {
    border-color: rgba(251, 146, 60, 0.28);
    background: linear-gradient(180deg, rgba(154, 52, 18, 0.28) 0%, var(--vp-c-bg-elv) 48%);
}

:global(html.dark) .home-pro {
    background:
        linear-gradient(180deg, rgba(154, 52, 18, 0.22) 0%, rgba(24, 24, 27, 0) 100%);
    border-top-color: rgba(251, 146, 60, 0.16);
}

:global(html.dark) .home-pro-grid article {
    border-color: rgba(251, 146, 60, 0.22);
    box-shadow: 0 8px 24px rgba(0, 0, 0, 0.24);
}

:global(html.dark) .home-protocol-pro-label {
    color: #fdba74;
}

:global(html.dark) .home-btn:hover {
    border-color: #fb923c;
    color: #fdba74;
}

:global(html.dark) .home-btn-brand {
    background: #ea580c;
}

:global(html.dark) .home-btn-brand:hover {
    background: #c2410c;
    color: #fff;
}

@media (min-width: 640px) {
    .home-stats {
        padding: 0 48px 32px;
    }

    .home-block,
    .home-pro-inner {
        padding-left: 48px;
        padding-right: 48px;
    }

    .home-compare-grid,
    .home-pro-grid {
        grid-template-columns: repeat(2, minmax(0, 1fr));
    }
}

@media (min-width: 960px) {
    .home-stats {
        grid-template-columns: repeat(4, minmax(0, 1fr));
        padding: 0 64px 32px;
    }

    .home-block,
    .home-pro-inner {
        padding-left: 64px;
        padding-right: 64px;
    }

    .home-block h2 {
        font-size: 32px;
    }

    .home-pro-grid {
        grid-template-columns: repeat(3, minmax(0, 1fr));
    }
}
</style>
