import Vue from 'vue'
import Socket from "./socket";
import { store } from './store'

export const api = new Vue({
  data() {
    return {
      _wsConnected: false
    }
  },
  methods: {
    send (message) {
      if (this._wsConnected) { Socket.send(JSON.stringify(message)) }
      else {
        setTimeout(() => {
          this.send(message)
        }, 200)
      }
    },
    getSettings () {
      this.send({ 'command': 'getSettings' })
    },
    getNetworks () {
      this.send({ 'command': 'getNetworks' })
    },
    getSchedule () {
      this.send({ 'command': 'getSchedule' })
    },
    getStatus () {
      this.send({ 'command': 'getStatus' })
    },
    getDuty () {
      this.send({ 'command': 'getDuty' })
    },
    setDuty (newDuty) {
      this.send({ 'command': 'setDuty', 'duty': newDuty })
    },
    setSettings () {
      this.send({ 'command': 'setSettings', leds: store.settings.leds, services: store.settings.services })
    },
    setSchedule () {
      this.send({ 'command': 'setSchedule', schedule: store.settings.schedule.items })
    },
    setNetworks () {
      this.send({ 'command': 'setNetworks', networks: store.settings.networks.items })
    },
    erase () {
      this.send({ 'command': 'erase' })
    },
    reboot () {
      this.send({ 'command': 'reboot' })
    }

  },
  created () {
    Socket.$on('connected', () => {
      this._wsConnected = true
    })
    Socket.$on('disconnected', () => {
      this._wsConnected = false
    })
  }
})
