<template>
  <div id="app">
    <navbar></navbar>
    <notification
      v-bind:message="notify.text"
      v-bind:type="notify.type"
      v-bind:showMessage="notify.visible" />
    <router-view/>
  </div>
</template>

<script>
import Socket from './socket'
import Navbar from '@/components/Ui/Navbar'
import { store, mutations } from './store'
import EventBus from './eventBus'
import Notification from '@/components/Ui/Notification'

export default {
  components: {
    Navbar, Notification
  },
  data () {
    return {
      ws: {
        url: '',
        connected: false
      },
      notify: {
        type: 'success',
        text: 'text',
        visible: false,
        autoHide: null,
        timeOut: 3000
      }
    }
  },
  methods: {
    rebootMicrocontroller () {
      Socket.send(JSON.stringify({ 'command': 'reboot' }))
    },
    hideMessage () {
      this.notify.text = ''
      this.notify.visible = false
    },
    showMessage (text, type) {
      this.notify.text = text
      this.notify.type = type
      this.notify.visible = true
      this.notify.autoHide = setInterval(this.hideMessage, this.notify.timeOut)
    }
  },
  mounted () {
    Socket.$on('connected', () => {
      this.ws.connected = true
      Socket.send(JSON.stringify({ 'command': 'getSettings' }))
    })
    Socket.$on('disconnected', () => {
      this.ws.connected = false
    })
    Socket.$on('message', (json) => {
      this.ws.connected = true
      console.log(json)

      /* Settings Response */
      if (json.leds && json.networks) {
        mutations.setSettings({ leds: json.leds, services: json.services })
        EventBus.$emit('settingsLoaded')
      }

      /* Led settings Response */
      else if (json.leds) {
        mutations.setSettings({ leds: json.leds })
        EventBus.$emit('ledLoaded')
      }

      /* Networks settings Response */
      else if (json.networks) {
        mutations.setSettings({ networks: { items: json.networks, capacity: json.capacity } })
        EventBus.$emit('networksLoaded')
      }

      /* Status Response */
      if (json.status) {
        mutations.setStatus(json.status)
        EventBus.$emit('statusLoaded')
      }

      /* Duty Response */
      if (json.duty) {
        EventBus.$emit('dutyReady', json.duty)
      }

      /* Schedule Response */
      if (json.schedule) {
        mutations.setSettings({ schedule: { items: json.schedule, capacity: json.capacity } })
        EventBus.$emit('scheduleLoaded')
      }

      /* Services settings Response */
      if (json.services) {
        mutations.setSettings({ services: json.services })
        EventBus.$emit('servicesLoaded')
      }

      // display Ui notification
      let response = json.response
      if (response) {
        switch (response) {
          case 'success':
            this.showMessage('Success', 'success')
            break
          default:
            break
        }
      }
    })
    EventBus.$on('setSettings', () => {
      Socket.send(JSON.stringify({ 'command': 'setSettings', leds: store.settings.leds, services: store.settings.services }))
    })
    EventBus.$on('setSchedule', () => {
      Socket.send(JSON.stringify({ 'command': 'setSchedule', schedule: store.settings.schedule.items }))
    })
    EventBus.$on('setNetworks', () => {
      Socket.send(JSON.stringify({ 'command': 'setNetworks', networks: store.settings.networks.items }))
    })
    EventBus.$on('getSettings', () => {
      Socket.send(JSON.stringify({ 'command': 'getSettings' }))
    })
    EventBus.$on('getNetworks', () => {
      Socket.send(JSON.stringify({ 'command': 'getNetworks' }))
    })
    EventBus.$on('getStatus', () => {
      Socket.send(JSON.stringify({ 'command': 'getStatus' }))
    })
    EventBus.$on('getSchedule', () => {
      Socket.send(JSON.stringify({ 'command': 'getSchedule' }))
    })
    EventBus.$on('reboot', () => {
      Socket.send(JSON.stringify({ 'command': 'reboot' }))
    })
    EventBus.$on('erase', () => {
      Socket.send(JSON.stringify({ 'command': 'erase' }))
    })
    EventBus.$on('getDuty', () => {
      Socket.send(JSON.stringify({ 'command': 'getDuty'}))
    })
    EventBus.$on('setDuty', (newDuty) => {
      Socket.send(JSON.stringify({ 'command': 'setDuty', 'duty': newDuty}))
    })
    EventBus.$on('message', (text, type) => {
      this.showMessage(text, type)
    })
  },
  destroyed () {
    if (this.notify.autoHide) clearInterval(this.autoHide)
  }
}
</script>

<style lang="scss">
#app {
  font-family: 'Avenir', Helvetica, Arial, sans-serif;
  -webkit-font-smoothing: antialiased;
  -moz-osx-font-smoothing: grayscale;
  text-align: center;
  color: #2c3e50;
}
#nav {
  padding: 30px;
  a {
    font-weight: bold;
    color: #2c3e50;
    &.router-link-exact-active {
      color: #42b983;
    }
  }
}
</style>
