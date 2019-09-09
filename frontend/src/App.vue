<template>
  <div
    id="app"
    class="has-background-dark has-text-light"
  >
    <navbar />
    <notification
      v-bind:message="notify.text"
      v-bind:type="notify.type"
      v-bind:show-message="notify.visible"
    />
    <router-view v-bind:class="{'is-disabled': isLoading}" />
    <div
      v-show="isLoading"
      class="loader"
    />
  </div>
</template>

<script>
import Notification from '@/components/Ui/Notification'
import Navbar from '@/components/Ui/Navbar'
import { mutations } from './store'
import { eventBus } from './eventBus'
import Socket from './socket'

export default {
  name: 'App',
  components: {
    Navbar, Notification
  },
  data () {
    return {
      ws: {
        url: '',
        connected: false
      },
      isLoading: false,
      notify: {
        type: 'success',
        text: 'text',
        visible: false,
        autoHide: null,
        timeOut: 3000
      }
    }
  },
  mounted () {
    console.log(`Started ${this.$appName}`)
    Socket.$on('connected', () => {
      this.ws.connected = true
      Socket.send(JSON.stringify({ command: 'getSettings' }))
    })
    Socket.$on('disconnected', () => {
      this.ws.connected = false
    })
    Socket.$on('message', (json) => {
      this.ws.connected = true
      console.log(json)

      if (json.leds) {
        /* Led settings Response */
        mutations.setSettings({ leds: json.leds })
        eventBus.$emit('loading', false)
        eventBus.$emit('ledLoaded')
      } else if (json.networks) {
        /* Networks settings Response */
        mutations.setSettings({ networks: { items: json.networks, capacity: json.capacity } })
        eventBus.$emit('loading', false)
        eventBus.$emit('networksLoaded')
      }

      /* Status Response */
      if (json.status) {
        mutations.setStatus(json.status)
        eventBus.$emit('loading', false)
        eventBus.$emit('statusLoaded')
      }

      /* Light Response */
      if (json.response === 'getLight') {
        if (json.brightness && json.duty && json.state && json.real_duty) {
          const light = Object.assign({}, {
            brightness: json.brightness,
            duty: json.duty,
            real_duty: json.real_duty,
            state: json.state
          })
          eventBus.$emit('loading', false)
          eventBus.$emit('lightReady', light)
        } else {
          /* Error load light status */
          console.log('Error loading LIGHT')
        }
      }

      /* Schedule Response */
      if (json.schedule) {
        mutations.setSettings({ schedule: { items: json.schedule, capacity: json.capacity } })
        eventBus.$emit('loading', false)
        eventBus.$emit('scheduleLoaded')
      }

      /* Services settings Response */
      if (json.services) {
        mutations.setSettings({ services: json.services })
        eventBus.$emit('loading', false)
        eventBus.$emit('servicesLoaded')
      }

      // display Ui notification
      const { response } = json
      if (response) {
        switch (response) {
          case 'success':
            eventBus.$emit('loading', false)
            this.showMessage('Success', 'success')
            break
          default: break
        }
      }
    })

    eventBus.$on('message', (text, type) => {
      this.showMessage(text, type)
    })

    eventBus.$on('loading', (show) => {
      this.isLoading = !!show
    })
  },
  destroyed () {
    if (this.notify.autoHide) clearInterval(this.autoHide)
    eventBus.$off()
  },
  methods: {
    rebootMicrocontroller () {
      Socket.send(JSON.stringify({ command: 'reboot' }))
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
  }
}
</script>
