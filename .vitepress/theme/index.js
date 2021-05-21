import DefaultTheme from 'vitepress/theme'
import DemoPlayer from '../../components/DemoPlayer.vue'
export default {
    ...DefaultTheme,
    enhanceApp({ app }) {
        app.component('DemoPlayer', DemoPlayer)
    }
}