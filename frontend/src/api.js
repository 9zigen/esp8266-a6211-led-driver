import Vue from 'vue'
import Socket from './socket'
import { store } from './store'
import { eventBus } from './eventBus'

export const api = new Vue({
  data () {
    return {
      wsConnected: false
    }
  },
  created () {
    Socket.$on('connected', () => {
      this._wsConnected = true
    })
    Socket.$on('disconnected', () => {
      this._wsConnected = false
    })
  },
  methods: {
    send (message) {
      eventBus.$emit('loading', true)
      if (this._wsConnected) {
        Socket.send(JSON.stringify(message))
      } else {
        setTimeout(() => {
          this.send(message)
        }, 200)
      }
    },
    getSettings () {
      this.send({ command: 'getSettings' })
    },
    getNetworks () {
      this.send({ command: 'getNetworks' })
    },
    getSchedule () {
      this.send({ command: 'getSchedule' })
    },
    getStatus () {
      this.send({ command: 'getStatus' })
    },
    getLight () {
      this.send({ command: 'getLight' })
    },
    setDuty (value) {
      this.send({ command: 'setDuty', duty: value })
    },
    setState (value) {
      this.send({ command: 'setState', state: value })
    },
    setBrightness (value) {
      this.send({ command: 'setBrightness', brightness: value })
    },
    setLeds () {
      this.send({ command: 'setLeds', leds: store.settings.leds })
    },
    setServices () {
      this.send({ command: 'setServices', services: store.settings.services })
    },
    setSchedule () {
      this.send({ command: 'setSchedule', schedule: store.settings.schedule.items })
    },
    setNetworks () {
      this.send({ command: 'setNetworks', networks: store.settings.networks.items })
    },
    erase () {
      this.send({ command: 'erase' })
    },
    reboot () {
      this.send({ command: 'reboot' })
    }

  }
})
