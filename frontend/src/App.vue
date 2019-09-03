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
  import Notification from '@/components/Ui/Notification'
  import Navbar from '@/components/Ui/Navbar'
  import { mutations } from './store'
  import { eventBus } from './eventBus'
  import Socket from './socket'


  export default {
    name: 'app',
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
        this.notify.visible = false
      },
      showMessage (text, type) {
        this.notify.text = text
        this.notify.type = type
        this.notify.visible = true
        clearTimeout(this.notify.autoHide)
        this.notify.autoHide = setInterval(this.hideMessage, this.notify.timeOut)
      }
    },
    mounted () {
      console.log('mounted App')
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
          eventBus.$emit('settingsLoaded')
        }

        /* Led settings Response */
        else if (json.leds) {
          mutations.setSettings({ leds: json.leds })
          eventBus.$emit('ledLoaded')
        }

        /* Networks settings Response */
        else if (json.networks) {
          mutations.setSettings({ networks: { items: json.networks, capacity: json.capacity } })
          eventBus.$emit('networksLoaded')
        }

        /* Status Response */
        if (json.status) {
          mutations.setStatus(json.status)
          eventBus.$emit('statusLoaded')
        }

        /* Duty Response */
        if (json.duty) {
          eventBus.$emit('dutyReady', json.duty)
        }

        /* Schedule Response */
        if (json.schedule) {
          mutations.setSettings({ schedule: { items: json.schedule, capacity: json.capacity } })
          eventBus.$emit('scheduleLoaded')
        }

        /* Services settings Response */
        if (json.services) {
          mutations.setSettings({ services: json.services })
          eventBus.$emit('servicesLoaded')
        }

        // display Ui notification
        let response = json.response
        if (response) {
          switch (response) {
            case 'success':
              this.showMessage('Success', 'success')
              break
            default: break
          }
        }
      })
      eventBus.$on('message', (text, type) => {
        this.showMessage(text, type)
      })
    },
    destroyed () {
      if (this.notify.autoHide) clearInterval(this.autoHide)
      eventBus.$off()
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
