import Vue from 'vue'
import Router from 'vue-router'
import Home from './views/Home'
import Settings from './views/Settings'
import About from './views/About'
import Schedule from './views/Schedule'
import Wifi from './views/Wifi'

Vue.use(Router)

export default new Router({
  mode: 'history',
  base: process.env.BASE_URL,
  linkActiveClass: 'is-active',
  routes: [
    {
      path: '/',
      name: 'home',
      component: Home,
      beforeEnter: (to, from, next) => {
        next()
      }
    },
    {
      path: '/schedule',
      name: 'schedule',
      component: Schedule,
      beforeEnter: (to, from, next) => {
        next()
      }
    },
    {
      path: '/wifi',
      name: 'wifi',
      component: Wifi,
      beforeEnter: (to, from, next) => {
        next()
      }
    },
    {
      path: '/settings',
      name: 'settings',
      component: Settings,
      beforeEnter: (to, from, next) => {
        next()
      }
    },
    {
      path: '/about',
      name: 'about',
      component: About
    }
  ]
})
