import Vue from 'vue'

export const store = Vue.observable({
  settings: {},
  status: {}
})

export const mutations = {
  setSettings (newSettings) {
    Object.assign(store.settings, newSettings)
  },
  setStatus (newStatus) {
    Object.assign(store.status, newStatus)
  }
}
