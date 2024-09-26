module.exports = {
    title: 'Jessibuca',
    description: '一款纯H5直播流播放器',
    themeConfig: {
        repo: "langhuihui/jessibuca/tree/v3",
        docsBranch: "v2",
        sidebar: "auto",
        nav: [
            {text: 'API', link: '/api'},
            {
                text: "More",
                items: [
                    {text: '文档', link: '/document'},
                    {text: 'Demo', link: '/demo'},
                    {text: '测试地址', link: '/test-url'},
                ]
            },
            {text: '播放器(Pro)', link: '/player-pro'},
            {
                text: "Pro-More",
                items: [
                    {text: '文档', link: '/pro'},
                    {text: 'Demo', link: '/pro-demo'},
                    {text: '帮助', link: '/pro-doc'},
                    {text: 'AI', link: '/pro-ai'},
                    {text: '扩展模块', link: '/pro-module'},
                    {text: '定制化服务', link: '/pro-more'},
                    {text: 'LVS分享视频', link: 'https://mp.weixin.qq.com/s/J6zXY1QOvmSqN4kLIaja_Q'},
                    {text: 'LVS分享PPT', link: 'https://jessibuca.com/pro-doc/2023-11-25-live-video-stack-wancheng.pdf'},
                ]
            },
            {text: '点播(ProVod)', link: '/player-pro-vod'},
            {
                text: "ProVod-More",
                items:[
                    {text: '文档', link: '/pro-vod'},
                ]
            },
            {
                text: 'http/https',
                items: [
                    {text: 'HTTP', link: 'http://jessibuca.monibuca.com/'},
                    {text: 'HTTPS', link: 'https://jessibuca.com/'},
                ]
            },
            {
                text: '不卡系列',
                items: [
                    {text: 'Monibuca', link: 'https://monibuca.com/'},
                    {text: 'Rebebuca', link: 'https://rebebuca.com/'}
                ]
            },
        ],
        logo: 'logo.png',
    },
    head: [
        ['meta',{'http-equiv':'origin-trial',content:'AhQB+uNRI7lww30oPK+0ZcGotIvuoHJL+NkkWOhqDdooY6+xnuiYmZli2SwlH1vkrKdB5WxMpsv5KRc/q9zFswoAAAB3eyJvcmlnaW4iOiJodHRwczovL2plc3NpYnVjYS5jb206NDQzIiwiZmVhdHVyZSI6IlVucmVzdHJpY3RlZFNoYXJlZEFycmF5QnVmZmVyIiwiZXhwaXJ5IjoxNzA5ODU1OTk5LCJpc1N1YmRvbWFpbiI6dHJ1ZX0='}],
        ['script', {src: 'https://cdn.wwads.cn/js/makemoney.js'}],
        ['script', {src: 'https://hm.baidu.com/hm.js?ed323be621396bd133c1c9b856d16dad'}],
        ['script', {src: '/jessibuca.js'}],
        ['script', {src: '/pro/js/jessibuca-pro-demo.js'}],
        ['script', {src: '/vconsole.js'}]
    ]
}
