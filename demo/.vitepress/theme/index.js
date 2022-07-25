import DefaultTheme from 'vitepress/theme'
import DemoPlayer from "../../components/DemoPlayer.vue"
import ProDemoPlayer from "../../components/ProDemoPlayer.vue"

export default {
    ...DefaultTheme,
    enhanceApp({app}) {
        app.component('DemoPlayer', DemoPlayer)
        app.component('ProDemoPlayer', ProDemoPlayer)
    }
}
