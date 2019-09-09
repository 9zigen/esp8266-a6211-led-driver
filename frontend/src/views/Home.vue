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
            <!-- Software status -->
            <div class="tile is-parent">
              <article class="tile is-child notification bg-notification is-primary">
                <p class="title">
                  Software
                </p>
                <p class="subtitle">
                  Services info
                </p>
                <div class="columns">
                  <div class="column has-text-left">
                    <ul>
                      <li>Time: {{ status.localTime }}</li>
                      <li>Uptime: {{ status.upTime }}</li>
                      <li>MQTT Server: {{ status.mqttService }}</li>
                      <li>NTP: {{ status.ntpService }}</li>
                    </ul>
                  </div>
                </div>
              </article>
            </div>
            <!-- Hardware Status -->
            <div class="tile is-parent">
              <article class="tile is-child notification bg-notification is-info">
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

        <div class="tile">
          <div class="tile is-vertical">
            <!-- Light Channel status -->
            <div class="tile is-parent">
              <article class="tile is-child notification bg-notification is-light">
                <p class="title">
                  Light Status
                </p>
                <div class="columns is-mobile is-multiline">
                  <div class="column is-narrow is-hidden-mobile is-hidden-tablet-only">
                    Channels Power:
                  </div>
                  <div
                    v-for="(led, index) in light.duty"
                    v-bind:key="index"
                    class="column is-narrow"
                  >
                    <span
                      class="tag is-light"
                      v-bind:style="{backgroundColor: sliderColor(index)}"
                    >{{ ledPower(index) }}</span>
                  </div>
                </div>
              </article>
            </div>

            <!-- Light toggle -->
            <div class="tile is-parent">
              <article class="tile is-child notification bg-notification is-warning">
                <p class="title">
                  Light Channel control
                </p>
                <div class="columns is-vcentered">
                  <div class="column is-12">
                    <div
                      v-for="(led, index) in light.duty"
                      v-bind:key="index"
                      class="field is-horizontal"
                    >
                      <div class="field-body">
                        <div class="field">
                          <div class="control">
                            <slider
                              v-model.number="light.duty[index]"
                              min="0"
                              max="255"
                              v-bind:color="sliderColor(index)"
                              @change="updateDuty"
                            />
                          </div>
                        </div>
                        <div class="field is-narrow">
                          <div class="control">
                            <toggle-switch
                              v-model="light.state[index]"
                              round
                              @change="updateState"
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
                              v-model.number="light.brightness"
                              min="0"
                              max="255"
                              @change="updateBrightness"
                            />
                          </div>
                        </div>
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

import { store } from '../store'
import { eventBus } from '../eventBus'
import { api } from '../api'

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
        wifiMode: 0,
        ipAddress: '',
        macAddress: '00:00:00:00:00:00',
        mqttService: '',
        ntpService: ''
      },
      light: {
        duty: [],
        state: [],
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
    eventBus.$once('statusLoaded', () => {
      this.getStatus()
      api.getLight()
    })

    eventBus.$on('lightReady', (newLight) => {
      /* set colors and channels power */
      if (store.settings.leds) {
        this.colors = store.settings.leds.map((value, index, array) => value.color)
        this.power = store.settings.leds.map((value, index, array) => value.channel_power)
      }
      /* load light status */
      Object.assign(this.light, newLight)
    })
  },
  destroyed () {
    eventBus.$off('lightReady')
    // clearInterval(this.refreshInterval)
  },
  methods: {
    channelControlButtonText (value) {
      return this.light.state === 1 ? 'ON' : 'OFF'
    },
    ledStatus (value) {
      if (value) { return `${parseInt(value * 100 / 255)}%` } return `OFF ${parseInt(value * 100 / 255)}%`
    },
    ledPower (index) {
      let string = ''
      const duty = this.light.real_duty

      if (duty[index] && this.power[index]) {
        const percent = duty[index] / 255
        const power = this.power[index]

        string += `${parseInt(percent * 100)}%`
        string += ` (${parseInt(percent * power)} Watts)`
        return string
      }
      return 'OFF '
    },
    getStatus () {
      if (store.status) {
        this.status = store.status
      }
    },
    restoreDevice () {
      api.erase()
    },
    rebootDevice () {
      api.reboot()
    },
    updateDuty () {
      if (this.light.duty.length > 0) {
        api.setDuty(this.light.duty)
        this.reloadLight()
      }
    },
    updateState () {
      if (this.light.state.length > 0) {
        api.setState(this.light.state)
        this.reloadLight()
      }
    },
    updateBrightness () {
      api.setBrightness(this.light.brightness)
      this.reloadLight()
    },
    sliderColor (index) {
      return this.colors[index]
    },
    reloadLight () {
      setTimeout(() => {
        api.getLight()
      }, 1500)
    }
  }
}
</script>

<style scoped>
  .action {
    margin: 8px !important;
  }
  /*@media screen and (max-width: 768px) {*/
    /*.is-fixed-bottom-touch {*/
      /*position: fixed;*/
    /*}*/
  /*}*/

</style>
