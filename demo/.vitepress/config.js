module.exports = {
    title: 'Jessibuca',
    description: '一款纯H5直播流播放器',
    themeConfig: {
        repo: "langhuihui/jessibuca",
        docsBranch: "v2",
        sidebar: "auto",
        nav:[
            { text: 'API', link: '/api' },
            { text: 'DEMO', link: '/demo' },
        ],
        logo: 'logo.png'
    },
    head: [
        ['script', {src: '/index.js'}]
    ]
}