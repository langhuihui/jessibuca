import { defineConfig } from 'vite';
import vue from '@vitejs/plugin-vue';
import vueJsx from '@vitejs/plugin-vue-jsx';
import Components from 'unplugin-vue-components/vite';
import { NaiveUiResolver } from 'unplugin-vue-components/resolvers';
import { resolve } from 'path';

// https://vitejs.dev/config/
export default defineConfig({
  mode: 'development',
  build: {
    outDir: '.vitepress/dist',
  },
  plugins: [
    vue(),
    vueJsx({
      include: [/\.[jt]sx$/]
    }),
    Components({
      resolvers: [NaiveUiResolver()]
    })
  ],
  resolve: {
    alias: {
      '@': resolve(__dirname, 'src'),
      'jv4-ui': resolve(__dirname, '../packages/ui/src'),
    }
  }
});