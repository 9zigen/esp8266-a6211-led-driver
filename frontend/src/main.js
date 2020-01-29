import Vue from 'vue'
import App from './App.vue'
import router from './router'

import GlobalOptions from '@/components/globalOptions'
import GlobalComponents from '@/components/globalComponents'

import './assets/style/custom.scss'

Vue.use(GlobalOptions)
Vue.use(GlobalComponents)

Vue.config.productionTip = false

new Vue({
  router,
  render: h => h(App)
}).$mount('#app')
