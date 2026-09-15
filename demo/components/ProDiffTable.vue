<template>
    <div class="pro-diff">
        <p class="pro-diff-lead">
            开源版覆盖基础直播拉流；Pro 在硬解、协议、回放、点播和业务控件上补齐。下面按模块列出全部对照项。
        </p>

        <div class="pro-diff-stats" aria-label="能力统计">
            <div class="pro-diff-stat">
                <strong>{{ total }}</strong>
                <span>对照项</span>
            </div>
            <div class="pro-diff-stat">
                <strong>{{ bothCount }}</strong>
                <span>双方都支持</span>
            </div>
            <div class="pro-diff-stat pro-diff-stat-pro">
                <strong>{{ proOnlyCount }}</strong>
                <span>仅 Pro</span>
            </div>
        </div>

        <div class="pro-diff-toolbar">
            <input
                v-model="keyword"
                class="pro-diff-search"
                type="search"
                placeholder="搜索能力，例如 WebRTC、水印、回放"
                aria-label="搜索对照项"
            />
            <div class="pro-diff-filters" role="tablist" aria-label="筛选对照范围">
                <button
                    type="button"
                    :class="{ 'is-active': filter === 'all' }"
                    @click="filter = 'all'"
                >全部</button>
                <button
                    type="button"
                    :class="{ 'is-active': filter === 'diff' }"
                    @click="filter = 'diff'"
                >仅看差异</button>
                <button
                    type="button"
                    :class="{ 'is-active': filter === 'both' }"
                    @click="filter = 'both'"
                >双方都支持</button>
            </div>
        </div>

        <div class="pro-diff-card">
            <div class="pro-diff-head">
                <span class="pro-diff-head-name">能力</span>
                <span class="pro-diff-head-cell">开源</span>
                <span class="pro-diff-head-cell">
                    <span class="pro-badge">Pro</span>
                </span>
            </div>

            <section
                v-for="group in visibleGroups"
                :key="group.title"
                class="pro-diff-group"
            >
                <div class="pro-diff-group-title">
                    <span>{{ group.title }}</span>
                    <small>{{ group.items.length }}</small>
                </div>
                <div
                    v-for="(item, index) in group.items"
                    :key="group.title + index"
                    class="pro-diff-row"
                    :class="{ 'is-pro-only': !item.o }"
                >
                    <div class="pro-diff-name" v-html="item.n"></div>
                    <div class="pro-diff-cell" :class="item.o ? 'is-yes' : 'is-no'">
                        <span
                            class="pro-diff-mark"
                            :title="item.o ? '支持' : '不支持'"
                            :aria-label="item.o ? '开源版支持' : '开源版不支持'"
                        >
                            <svg v-if="item.o" viewBox="0 0 16 16" aria-hidden="true">
                                <path d="M6.4 11.2 3.2 8l1.2-1.2 2 2 5.2-5.2L13 5.2 6.4 11.2z"/>
                            </svg>
                            <svg v-else viewBox="0 0 16 16" aria-hidden="true">
                                <path d="M4 7.25h8v1.5H4z"/>
                            </svg>
                        </span>
                    </div>
                    <div class="pro-diff-cell is-yes is-pro">
                        <span class="pro-diff-mark" title="支持" aria-label="Pro 支持">
                            <svg viewBox="0 0 16 16" aria-hidden="true">
                                <path d="M6.4 11.2 3.2 8l1.2-1.2 2 2 5.2-5.2L13 5.2 6.4 11.2z"/>
                            </svg>
                        </span>
                    </div>
                </div>
            </section>

            <p v-if="!visibleGroups.length" class="pro-diff-empty">没有匹配的对照项，试试换个关键词。</p>
        </div>
    </div>
</template>

<script>
import groups from './pro-diff-data.js'

export default {
    name: 'ProDiffTable',
    data() {
        return {
            keyword: '',
            filter: 'all',
            groups
        }
    },
    computed: {
        total() {
            return this.groups.reduce((sum, group) => sum + group.items.length, 0)
        },
        bothCount() {
            return this.groups.reduce((sum, group) => sum + group.items.filter((item) => item.o).length, 0)
        },
        proOnlyCount() {
            return this.total - this.bothCount
        },
        visibleGroups() {
            const keyword = this.keyword.trim().toLowerCase()
            return this.groups
                .map((group) => ({
                    title: group.title,
                    items: group.items.filter((item) => {
                        if (this.filter === 'diff' && item.o) return false
                        if (this.filter === 'both' && !item.o) return false
                        if (!keyword) return true
                        return item.n.replace(/<[^>]+>/g, '').toLowerCase().includes(keyword)
                    })
                }))
                .filter((group) => group.items.length)
        }
    }
}
</script>

<style scoped>
.pro-diff {
    --pro-diff-head-h: 44px;
    --pro-diff-col: 54px;
    margin: 4px 0 36px;
}

.pro-diff-lead {
    margin: 0 0 16px;
    color: var(--vp-c-text-2);
    font-size: 15px;
    line-height: 1.7;
}

.pro-diff-stats {
    display: grid;
    grid-template-columns: repeat(3, minmax(0, 1fr));
    gap: 8px;
    margin-bottom: 14px;
}

.pro-diff-stat {
    display: flex;
    flex-direction: column;
    gap: 2px;
    min-width: 0;
    padding: 12px 10px;
    border: 1px solid var(--vp-c-divider);
    border-radius: 12px;
    background: var(--vp-c-bg-elv);
}

.pro-diff-stat strong {
    font-size: 22px;
    font-weight: 700;
    letter-spacing: -0.03em;
    line-height: 1.15;
    color: var(--vp-c-text-1);
}

.pro-diff-stat span {
    color: var(--vp-c-text-2);
    font-size: 12px;
    font-weight: 600;
    white-space: nowrap;
}

.pro-diff-stat-pro {
    border-color: rgba(234, 88, 12, 0.28);
    background: #fff7ed;
}

.pro-diff-stat-pro strong {
    color: #c2410c;
}

.pro-diff-toolbar {
    display: flex;
    flex-direction: column;
    gap: 10px;
    margin-bottom: 12px;
}

.pro-diff-search {
    width: 100%;
    height: 40px;
    padding: 0 14px;
    border: 1px solid var(--vp-c-divider);
    border-radius: 12px;
    background: var(--vp-c-bg-elv);
    color: var(--vp-c-text-1);
    font-size: 16px;
}

.pro-diff-search:focus {
    outline: none;
    border-color: var(--vp-c-brand-1);
    box-shadow: 0 0 0 3px var(--vp-c-brand-soft);
}

.pro-diff-filters {
    display: flex;
    width: 100%;
    padding: 3px;
    border: 1px solid var(--vp-c-divider);
    border-radius: 999px;
    background: var(--vp-c-bg-soft);
}

.pro-diff-filters button {
    flex: 1;
    height: 32px;
    padding: 0 8px;
    border: 0;
    border-radius: 999px;
    background: transparent;
    color: var(--vp-c-text-2);
    font-size: 13px;
    font-weight: 600;
    white-space: nowrap;
    cursor: pointer;
}

.pro-diff-filters button.is-active {
    background: #c2410c;
    color: #fff;
    box-shadow: 0 4px 10px rgba(194, 65, 12, 0.22);
}

.pro-diff-card {
    overflow: hidden;
    border: 1px solid var(--vp-c-divider);
    border-radius: 16px;
    background: var(--vp-c-bg-elv);
    box-shadow: 0 10px 28px rgba(15, 23, 42, 0.05);
}

.pro-diff-head,
.pro-diff-row {
    display: grid;
    grid-template-columns: minmax(0, 1fr) var(--pro-diff-col) var(--pro-diff-col);
    align-items: center;
    column-gap: 0;
}

.pro-diff-head {
    z-index: 3;
    min-height: var(--pro-diff-head-h);
    padding: 0 6px 0 14px;
    background: #f8fafc;
    border-bottom: 1px solid var(--vp-c-divider);
    color: var(--vp-c-text-3);
    font-size: 12px;
    font-weight: 700;
}

.pro-diff-head-name {
    letter-spacing: 0.04em;
}

.pro-diff-head-cell,
.pro-diff-cell {
    display: flex;
    align-items: center;
    justify-content: center;
    height: 100%;
}

.pro-diff-head-cell + .pro-diff-head-cell,
.pro-diff-cell + .pro-diff-cell {
    box-shadow: inset 1px 0 0 var(--vp-c-divider);
}

.pro-badge {
    display: inline-flex;
    align-items: center;
    justify-content: center;
    height: 18px;
    padding: 0 6px;
    border-radius: 999px;
    background: linear-gradient(135deg, #ea580c 0%, #c2410c 100%);
    color: #fff;
    font-size: 10px;
    font-weight: 800;
    letter-spacing: 0.08em;
    text-transform: uppercase;
    line-height: 1;
}

.pro-diff-group-title {
    display: flex;
    align-items: center;
    gap: 8px;
    min-height: 36px;
    padding: 0 14px;
    background: #f1f5f9;
    border-top: 1px solid var(--vp-c-divider);
    border-bottom: 1px solid var(--vp-c-divider);
    color: var(--vp-c-text-1);
    font-size: 13px;
    font-weight: 700;
}

.pro-diff-group:first-of-type .pro-diff-group-title {
    border-top: 0;
}

.pro-diff-group-title small {
    display: inline-flex;
    align-items: center;
    justify-content: center;
    min-width: 20px;
    height: 18px;
    padding: 0 6px;
    border-radius: 999px;
    background: rgba(15, 23, 42, 0.06);
    color: var(--vp-c-text-2);
    font-size: 11px;
    font-weight: 700;
}

.pro-diff-row {
    min-height: 44px;
    padding: 8px 6px 8px 14px;
    background: var(--vp-c-bg-elv);
}

.pro-diff-row + .pro-diff-row {
    box-shadow: inset 0 1px 0 var(--vp-c-divider);
}

.pro-diff-row:hover {
    background: var(--vp-c-bg-soft);
}

.pro-diff-name {
    min-width: 0;
    padding-right: 8px;
    color: var(--vp-c-text-1);
    font-size: 13.5px;
    line-height: 1.5;
    overflow-wrap: break-word;
}

.pro-diff-name :deep(a) {
    font-weight: 600;
    text-decoration: none;
    margin-left: 2px;
}

.pro-diff-name :deep(a:hover) {
    text-decoration: underline;
}

.pro-diff-mark {
    display: inline-flex;
    align-items: center;
    justify-content: center;
    width: 22px;
    height: 22px;
    border-radius: 999px;
}

.pro-diff-cell.is-yes .pro-diff-mark {
    background: rgba(22, 163, 74, 0.12);
    color: #15803d;
}

.pro-diff-cell.is-pro .pro-diff-mark {
    background: rgba(234, 88, 12, 0.12);
    color: #c2410c;
}

.pro-diff-cell.is-no .pro-diff-mark {
    background: rgba(148, 163, 184, 0.14);
    color: #94a3b8;
}

.pro-diff-mark svg {
    width: 14px;
    height: 14px;
    fill: currentColor;
}

.pro-diff-empty {
    margin: 0;
    padding: 28px 16px;
    color: var(--vp-c-text-2);
    text-align: center;
}

:global(html.dark) .pro-diff-stat-pro {
    background: rgba(154, 52, 18, 0.28);
    border-color: rgba(251, 146, 60, 0.28);
}

:global(html.dark) .pro-diff-stat-pro strong {
    color: #fdba74;
}

:global(html.dark) .pro-diff-head {
    background: #1f1f23;
}

:global(html.dark) .pro-diff-group-title {
    background: #27272a;
}

:global(html.dark) .pro-diff-group-title small {
    background: rgba(255, 255, 255, 0.08);
}

:global(html.dark) .pro-diff-cell.is-yes .pro-diff-mark {
    background: rgba(74, 222, 128, 0.16);
    color: #86efac;
}

:global(html.dark) .pro-diff-cell.is-pro .pro-diff-mark {
    background: rgba(251, 146, 60, 0.18);
    color: #fdba74;
}

:global(html.dark) .pro-diff-cell.is-no .pro-diff-mark {
    background: rgba(148, 163, 184, 0.12);
    color: #64748b;
}

:global(html.dark) .pro-diff-filters {
    background: rgba(255, 255, 255, 0.04);
}

:global(html.dark) .pro-diff-filters button.is-active {
    background: #ea580c;
}

@media (min-width: 640px) {
    .pro-diff {
        --pro-diff-col: 72px;
    }

    .pro-diff-toolbar {
        flex-direction: row;
        align-items: center;
    }

    .pro-diff-search {
        flex: 1;
        font-size: 14px;
    }

    .pro-diff-filters {
        width: auto;
        flex: none;
    }

    .pro-diff-filters button {
        flex: none;
        padding: 0 12px;
    }

    .pro-diff-head,
    .pro-diff-row,
    .pro-diff-group-title {
        padding-left: 18px;
    }

    .pro-diff-head,
    .pro-diff-row {
        padding-right: 10px;
    }

    .pro-diff-name {
        font-size: 14px;
    }
}
</style>
