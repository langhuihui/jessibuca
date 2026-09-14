import { defineConfig } from 'vitepress'

export default defineConfig({
    title: 'Jessibuca',
    description: '一款纯H5直播流播放器',
    appearance: true,
    ignoreDeadLinks: true,
    themeConfig: {
        logo: '/logo.png',
        outline: {
            level: 'deep',
            label: '目录'
        },
        aside: 'left',
        darkModeSwitchLabel: '外观',
        lightModeSwitchTitle: '切换到浅色模式',
        darkModeSwitchTitle: '切换到深色模式',
        footer: {
            message: 'GPL Licensed',
            copyright: 'Copyright © 2020-present dexter · <a href="https://beian.miit.gov.cn/" target="_blank" rel="noreferrer">苏ICP备2023025807号-2</a>'
        },
        search: {
            provider: 'local',
            options: {
                translations: {
                    button: {
                        buttonText: '搜索',
                        buttonAriaLabel: '搜索文档'
                    },
                    modal: {
                        displayDetails: '显示详细列表',
                        resetButtonTitle: '清除查询条件',
                        backButtonTitle: '关闭搜索',
                        noResultsText: '没有找到相关结果',
                        footer: {
                            selectText: '选择',
                            navigateText: '切换',
                            closeText: '关闭'
                        }
                    }
                },
                miniSearch: {
                    options: {
                        tokenize: (term) => {
                            if (typeof term !== 'string') return []
                            return term.toLowerCase().match(/[\u4e00-\u9fff]|[a-z0-9]+/g) || []
                        }
                    },
                    searchOptions: {
                        fuzzy: 0.2,
                        prefix: true
                    }
                }
            }
        },
        socialLinks: [
            { icon: 'github', link: 'https://github.com/langhuihui/jessibuca/tree/v3' }
        ],
        nav: [
            { text: 'API', link: '/api' },
            {
                text: 'More',
                items: [
                    { text: '文档', link: '/document' },
                    { text: 'Demo', link: '/demo' },
                    { text: '测试地址', link: '/test-url' },
                    { text: 'HTTP(ws) 流媒体下载器', link: 'https://jessibuca.com/download-utils.html' },
                    { text: '文章分享系列', link: 'https://juejin.cn/user/1345457961842126/posts' },
                ]
            },
            { text: '播放器(Pro)', link: '/player-pro' },
            {
                text: 'Pro-More',
                items: [
                    { text: '文档', link: '/pro' },
                    { text: 'Demo', link: '/pro-demo' },
                    { text: '帮助', link: '/pro-doc' },
                    { text: 'AI', link: '/pro-ai' },
                    { text: '扩展模块', link: '/pro-module' },
                    { text: '定制化服务', link: '/pro-more' },
                    { text: 'Pro文档介绍', link: 'https://jessibuca.com/pro-doc/JessibucaProDoc.pdf' },
                    { text: 'LVS分享视频', link: 'https://mp.weixin.qq.com/s/J6zXY1QOvmSqN4kLIaja_Q' },
                    { text: 'LVS分享PPT', link: 'https://jessibuca.com/pro-doc/2023-11-25-live-video-stack-wancheng.pdf' },
                ]
            },
            { text: '点播(ProVod)', link: '/player-pro-vod' },
            {
                text: 'ProVod-More',
                items: [
                    { text: '文档', link: '/pro-vod' },
                ]
            },
            {
                text: 'http/https',
                items: [
                    { text: 'HTTP', link: 'http://jessibuca.monibuca.com/' },
                    { text: 'HTTPS', link: 'https://jessibuca.com/' },
                ]
            },
            {
                text: '不卡系列',
                items: [
                    { text: 'Monibuca(纯 Go 开源一站式流媒体服务器开发框架)', link: 'https://monibuca.com/' },
                    { text: 'Rebebuca(强大的运行配置管理工具)', link: 'https://rebebuca.com/' }
                ]
            },
            {
                text: '友情链接',
                items: [
                    { text: 'Mediainsight(媒体格式在线分析)', link: 'https://mediainsight360.com/' },
                ]
            }
        ],
    },
    head: [
        ['meta', { 'http-equiv': 'origin-trial', content: 'AhQB+uNRI7lww30oPK+0ZcGotIvuoHJL+NkkWOhqDdooY6+xnuiYmZli2SwlH1vkrKdB5WxMpsv5KRc/q9zFswoAAAB3eyJvcmlnaW4iOiJodHRwczovL2plc3NpYnVjYS5jb206NDQzIiwiZmVhdHVyZSI6IlVucmVzdHJpY3RlZFNoYXJlZEFycmF5QnVmZmVyIiwiZXhwaXJ5IjoxNzA5ODU1OTk5LCJpc1N1YmRvbWFpbiI6dHJ1ZX0=' }],
        ['script', { src: 'https://cdn.wwads.cn/js/makemoney.js' }],
        ['script', { src: 'https://hm.baidu.com/hm.js?ed323be621396bd133c1c9b856d16dad' }],
        ['script', { src: '/jessibuca.js' }],
        ['script', { src: '/pro/js/jessibuca-pro-vr-demo.js' }],
        ['script', { src: '/pro/js/jessibuca-pro-demo.js' }],
        ['script', { src: '/vconsole.js' }]
    ]
})
