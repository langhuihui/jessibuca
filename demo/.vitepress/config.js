module.exports = {
    title: 'Jessibuca',
    description: '一款纯H5直播流播放器',
    themeConfig: {
        repo: "langhuihui/jessibuca/tree/v3",
        docsBranch: "v2",
        sidebar: "auto",
        nav: [
            {text: 'API', link: '/api'},
            {text: '2.X', link: 'http://jessibuca.monibuca.com/'},
        ],
        logo: 'logo.png',
    },
    head: [
        ['script', {src: '/jessibuca.js'}]
    ]
}
