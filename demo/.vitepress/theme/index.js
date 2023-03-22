import DefaultTheme from 'vitepress/theme'
import ElementPlus from 'element-plus'
import 'element-plus/dist/index.css'
import DemoPlayer from "../../components/DemoPlayer.vue"
import ProDemoPlayer from "../../components/ProDemoPlayer.vue"
import Rice from '../../components/Rice.vue';

export default {
    ...DefaultTheme,
    enhanceApp({app}) {
        app.use(ElementPlus);
        app.component('DemoPlayer', DemoPlayer)
        app.component('ProDemoPlayer', ProDemoPlayer)
        app.component('Rice', Rice)
    }
}
