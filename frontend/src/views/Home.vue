<template>
  <div class="container">
    <!-- Page Title -->
    <div class="tile is-ancestor is-marginless">
      <div class="tile is-parent">
        <div class="tile is-child text-left">
          <h1 class="title">
            Dashboard
          </h1>
        </div>
      </div>
    </div>

    <div class="tile is-ancestor is-marginless">
      <!-- Wrapper -->
      <div class="tile is-vertical">
        <div class="tile">
          <!-- left col -->
          <div class="tile is-vertical">
            <!-- Schedule chart -->
            <div class="tile is-parent">
              <article class="tile is-child notification bg-notification is-light">
                <div class="content">
                  <p class="title">
                    Light Schedule
                  </p>
                  <div class="content">
                    <schedule-chart />
                  </div>
                </div>
              </article>
            </div>
          </div>
          <!-- Right col -->
          <div class="tile is-vertical">
            <!-- Light toggle -->
            <div class="tile is-parent">
              <article class="tile is-child notification bg-notification is-warning">
                <p class="title">
                  Light Channel control
                </p>
                <div class="columns is-vcentered">
                  <div class="column is-12">
                    <div
                      v-for="(led, index) in status.channels"
                      v-bind:key="index"
                      class="field is-horizontal"
                    >
                      <div class="field-body">
                        <div class="field">
                          <div class="control">
                            <slider
                              v-model.number="status.channels[index]"
                              min="0"
                              max="100"
                              v-bind:color="sliderColor(index)"
                              @change="setDuty"
                            />
                          </div>
                        </div>
                      </div>
                    </div>
                    <div class="field is-horizontal">
                      <div class="field-body">
                        <div class="field">
                          <label>Brightness</label>
                          <div class="control">
                            <slider
                              v-model.number="status.brightness"
                              min="0"
                              max="100"
                              @change="setBrightness"
                            />
                          </div>
                        </div>
                      </div>
                    </div>
                  </div>
                </div>
              </article>
            </div>
            <!-- Light Channel status -->
            <div class="tile is-parent">
              <article class="tile is-child notification bg-notification is-light">
                <p class="title">
                  Light Status
                </p>
                <p class="subtitle">
                  Channels Power
                </p>
                <div class="columns is-mobile is-multiline">
                  <div
                    v-for="(led, index) in status.channels"
                    v-bind:key="index"
                    class="column is-narrow"
                  >
                    <div
                      class="notification is-light is-medium"
                      v-bind:style="{backgroundColor: sliderColor(index)}"
                    >
                      #{{ index + 1 }} <br> {{ ledPower(index) }}
                    </div>
                  </div>
                </div>
              </article>
            </div>
          </div>
        </div>

        <div class="tile">
          <div class="tile is-vertical">
            <!-- Hardware Status -->
            <div class="tile is-parent">
              <article class="tile is-child notification bg-notification is-primary">
                <p class="title">
                  Hardware
                </p>
                <p class="subtitle">
                  Device info
                </p>
                <div class="columns">
                  <div class="column has-text-left">
                    <ul>
                      <li>Chip ID: {{ status.chipId }}</li>
                      <li>Free Heap: {{ status.freeHeap }}</li>
                      <li>VCC: {{ status.vcc }}</li>
                      <li>WIFI: {{ wifiCurrentMode }}</li>
                      <li>IP Address: {{ status.ipAddress }}</li>
                      <li>MAC: {{ status.macAddress }}</li>
                    </ul>
                  </div>
                  <div class="column has-text-left">
                    <ul>
                      <li>Time: {{ status.localTime }}</li>
                      <li>Uptime: {{ status.upTime }}</li>
                      <li>MQTT Server: {{ status.mqttService }}</li>
                      <li>NTP: {{ status.ntpService }}</li>
                    </ul>
                  </div>
                </div>
                <div class="columns">
                  <div class="column has-text-left">
                    <div class="field is-grouped">
                      <div class="control">
                        <a
                          class="button is-warning"
                          @click="rebootDevice"
                        >Restart</a>
                        <p class="help">
                          Reboot device
                        </p>
                      </div>
                      <div class="control">
                        <a
                          class="button is-danger"
                          @click="restoreDevice"
                        >Factory Reset</a>
                        <p class="help">
                          Restore initial configuration
                        </p>
                      </div>
                    </div>
                  </div>
                </div>
              </article>
            </div>
          </div>
        </div>
      </div>
    </div>
  </div>
</template>

<script>

import { eventBus } from '../eventBus'
import { http } from '../http'

export default {
  name: 'Home',
  data () {
    return {
      status: {
        upTime: '',
        localTime: '',
        chipId: 0,
        freeHeap: 0,
        vcc: 0,
        wifiMode: '',
        ipAddress: '',
        macAddress: '00:00:00:00:00:00',
        mqttService: '',
        ntpService: '',
        channels: [0, 0, 0, 0, 0],
        brightness: 0
      },
      colors: [],
      power: [],
      refreshInterval: null,
      isLoading: true
    }
  },
  computed: {
    wifiCurrentMode () {
      const WiFiMode = ['OFF', 'Station', 'Access Point', 'AP+STA']
      if (this.status.wifiMode > WiFiMode.length) { return }

      return WiFiMode[this.status.wifiMode]
    }
  },
  mounted () {
    this.requestData()
  },
  methods: {
    async requestData () {
      eventBus.$emit('loading', true)
      try {
        /* Device status */
        let status = await http.get('/status')
        this.status = status.data

        /* Leds config */
        let responseLeds = await http.get('/config/leds')
        this.colors = responseLeds.data.leds.map((value, index, array) => value.color)
        this.power = responseLeds.data.leds.map((value, index, array) => value.power)
      } catch (e) {
        eventBus.$emit('message', e, 'danger')
      }
      eventBus.$emit('loading', false)
    },
    ledStatus (value) {
      if (value) { return `${parseInt(value)}%` } return `OFF ${parseInt(value)}%`
    },
    ledPower (index) {
      let string = ''
      const channels = this.status.channels

      if (this.power[index] === 0) {
        return 'POWER not SET '
      }

      if (channels[index]) {
        const percent = channels[index] / 100
        const power = this.power[index]

        string += `${parseInt(percent * 100)}%`
        string += ` (${parseInt(percent * power)} Watts)`
        return string
      }
      return 'OFF '
    },
    async restoreDevice () {
      try {
        let response = await http.get('/factory')
        if (response.data.success) { eventBus.$emit('message', 'Factory Restoring...', 'success') }
      } catch (e) {
        eventBus.$emit('message', e, 'danger')
      }
    },
    async rebootDevice () {
      try {
        let response = await http.get('/reboot')
        if (response.data.success) { eventBus.$emit('message', 'Rebooting...', 'success') }
      } catch (e) {
        eventBus.$emit('message', e, 'danger')
      }
    },
    async setDuty () {
      if (this.status.channels.length > 0) {
        try {
          /* Set New Duty value */
          let response = await http.post('/set/duty', { duty: this.status.channels })
          this.$set(this.status, 'channels', response.data.duty)
        } catch (e) {
          eventBus.$emit('message', e, 'danger')
        }
      }
    },
    async setBrightness () {
      if (this.status.brightness !== undefined) {
        try {
          /* Set New Duty value */
          let response = await http.post('/set/brightness', { brightness: this.status.brightness })
          this.$set(this.status, 'brightness', response.data)
        } catch (e) {
          eventBus.$emit('message', e, 'danger')
        }
      }
    },
    sliderColor (index) {
      return this.colors[index]
    }
  }
}
</script>
