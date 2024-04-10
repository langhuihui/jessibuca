import { defineConfig } from 'vite'
import vue from '@vitejs/plugin-vue'
import Components from 'unplugin-vue-components/vite'
import { NaiveUiResolver } from 'unplugin-vue-components/resolvers'

// https://vitejs.dev/config/
export default defineConfig({
  mode: 'development',
  build: {
    outDir: '.vitepress/dist',
  },
  plugins: [
    vue(),
    Components({
      resolvers: [NaiveUiResolver()]
    })
  ]
})