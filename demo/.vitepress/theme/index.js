import DefaultTheme from 'vitepress/theme'
import { h } from 'vue'
import ElementPlus from 'element-plus'
import 'element-plus/dist/index.css'
import './custom.css'
import DemoPlayer from '../../components/DemoPlayer.vue'
import ProDemoPlayer from '../../components/ProDemoPlayer.vue'
import Rice from '../../components/Rice.vue'
import HomeExtras from '../../components/HomeExtras.vue'
import PlayerPageNotes from '../../components/PlayerPageNotes.vue'

export default {
    extends: DefaultTheme,
    Layout() {
        return h(DefaultTheme.Layout, null, {
            'home-hero-after': () => h(HomeExtras, { part: 'stats' }),
            'home-features-after': () => h(HomeExtras, { part: 'body' })
        })
    },
    enhanceApp({ app }) {
        if (!import.meta.env.SSR) {
            app.use(ElementPlus)
        }
        app.component('DemoPlayer', DemoPlayer)
        app.component('ProDemoPlayer', ProDemoPlayer)
        app.component('Rice', Rice)
        app.component('HomeExtras', HomeExtras)
        app.component('PlayerPageNotes', PlayerPageNotes)
    }
}
