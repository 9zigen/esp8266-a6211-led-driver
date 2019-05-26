import Vue from 'vue'
import App from './App.vue'
import router from './router'
import VueApexCharts from 'vue-apexcharts'
import VueTheMask from 'vue-the-mask'

import './assets/style/custom.scss'

Vue.use(VueApexCharts)
Vue.use(VueTheMask)

Vue.config.productionTip = false

new Vue({
  router,
  render: h => h(App)
}).$mount('#app')
