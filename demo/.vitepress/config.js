module.exports = {
    title: 'Jessibuca',
    description: '一款纯H5直播流播放器',
    themeConfig: {
        repo: "langhuihui/jessibuca/tree/v3",
        docsBranch: "v2",
        sidebar: "auto",
        nav: [
            {text: 'API', link: '/api'},
            {text: 'DEMO', link: '/demo' },
            {text: 'Document', link: '/document'},
            {text: 'PRO', link: '/pro'},
            {text: 'PRO-Player', link: '/player-pro'},
            {text: 'PRO-AI', link: '/pro-ai' },
            {text: 'PRO-DEMO', link: '/pro-demo' },
            {text: 'HTTP', link: 'http://jessibuca.monibuca.com/'},
            {text: 'HTTPS', link: 'https://j.m7s.live/'},
            {text: 'Monibuca', link: 'https://m7s.live/'},
        ],
        logo: 'logo.png',
    },
    head: [
        ['script', {src: 'https://hm.baidu.com/hm.js?ed323be621396bd133c1c9b856d16dad'}],
        ['script', {src: '/jessibuca.js'}],
        ['script', {src: '/pro/jessibuca-pro-demo.js'}],
        ['script', {src: '/vconsole.js'}]
    ]
}
