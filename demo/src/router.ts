import { createRouter, createWebHistory } from 'vue-router';
import UI from './components/UI.vue';
import TimeRangeDemo from './components/TimeRangeDemo.vue';

const router = createRouter({
  history: createWebHistory(),
  routes: [
    {
      path: '/',
      component: UI
    },
    {
      path: '/time-range',
      component: TimeRangeDemo
    }
  ]
});

export default router; 