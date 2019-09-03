<template>
  <div class="container">
    <h1 class="title is-5">Services Settings</h1>
    <h2 class="subtitle is-7">MQTT</h2>
    <div class="columns is-centered">
      <div class="column is-10-mobile is-offset-1-mobile is-10">
        <div class="columns is-vcentered">
          <div class="column is-12">
            <div class="field is-horizontal">
              <div class="field-label is-normal">
                <label class="label">Identity</label>
              </div>
              <div class="field-body">
                <div class="field">
                  <div class="control">
                    <input class="input" type="text" placeholder="Hostname" v-model="services.hostname">
                  </div>
                  <p class="help has-text-left">hostname</p>
                </div>
              </div>
            </div>
            <div class="field is-horizontal">
              <div class="field-label is-normal">
                <label class="label">NTP</label>
              </div>
              <div class="field-body">
                <div class="field">
                  <div class="control">
                    <toggle-switch v-model="services.enable_ntp_service"></toggle-switch>
                  </div>
                  <p class="help has-text-left">Enable NTP</p>
                </div>
                <div class="field">
                  <div class="control">
                    <input class="input" type="text" placeholder="NTP Server Name" v-model="services.ntp_server_name">
                  </div>
                  <p class="help">Server Name</p>
                </div>
                <div class="field">
                  <div class="control">
                    <input class="input" type="text" placeholder="UTC Offset" v-model="services.utc_offset_minutes">
                  </div>
                  <p class="help">UTC Offset</p>
                </div>
                <div class="field">
                  <div class="control has-text-centered">
                    <toggle-switch v-model="services.ntp_dst"></toggle-switch>
                  </div>
                  <p class="help">Daylight saving time</p>
                </div>
              </div>
            </div>
            <div class="field is-horizontal">
              <div class="field-label is-normal">
                <label class="label">MQTT</label>
              </div>
              <div class="field-body">
                <div class="field">
                  <div class="control">
                    <toggle-switch v-model="services.enable_mqtt_service"></toggle-switch>
                  </div>
                  <p class="help has-text-left">Enable MQTT</p>
                </div>
                <div class="field">
                  <div class="control">
                    <input class="input" type="text" placeholder="MQTT IP Address" v-model="services.mqtt_ip_address" v-mask="'###.###.###.###'">
                  </div>
                  <p class="help">MQTT Server IP Address</p>
                </div>
                <div class="field">
                  <div class="control">
                    <input class="input" type="text" placeholder="MQTT Server Port" v-model="services.mqtt_port">
                  </div>
                  <p class="help">MQTT Server Port</p>
                </div>
                <div class="field">
                  <div class="control">
                    <input class="input" type="text" placeholder="MQTT QoS" v-model="services.mqtt_qos">
                  </div>
                  <p class="help">MQTT QoS</p>
                </div>
              </div>
            </div>
            <div class="field is-horizontal">
              <div class="field-label is-normal">
                <label class="label"></label>
              </div>
              <div class="field-body">
                <div class="field">
                  <div class="control">
                    <input class="input" type="text" placeholder="User" v-model="services.mqtt_user">
                  </div>
                  <p class="help">MQTT User</p>
                </div>
                <div class="field">
                  <div class="control">
                    <input class="input" type="text" placeholder="Password" v-model="services.mqtt_password">
                  </div>
                  <p class="help">MQTT Password</p>
                </div>
              </div>
            </div>
          </div>
        </div>
      </div>
    </div>

    <h1 class="title is-5">LED Settings</h1>
    <h2 class="subtitle is-7">LED Colors</h2>
    <div class="columns is-centered">
      <div class="column is-10-mobile is-offset-1-mobile is-8">
        <div class="columns is-vcentered" v-for="led in leds" :key="led.id">
          <div class="column is-12">
            <div class="field is-horizontal">
              <div class="field-label is-normal">
                <label class="label">LED {{led.id + 1}}</label>
              </div>
              <div class="field-body">
                <div class="field">
                  <div class="control">
                    <div class="select">
                      <select name="color" v-model="led.color">
                        <option v-for="option in ledOptions" v-bind:value="option.value">
                          {{ option.text }}
                        </option>
                      </select>
                    </div>
                    <p class="help">Color</p>
                  </div>
                </div>

                <div class="field">
                  <div class="control">
                    <input class="input" type="text" placeholder="default duty" v-model="led.default_duty">
                  </div>
                  <p class="help">Default Duty</p>
                </div>
              </div>
            </div>
          </div>
        </div>
      </div>
    </div>

    <div class="columns is-centered">
      <div class="field is-grouped is-grouped-centered">
        <p class="control">
          <a class="button is-primary" v-on:click="saveSettings">
            Submit
          </a>
        </p>
        <p class="control">
          <a class="button is-light" v-on:click="loadSettings">
            Cancel
          </a>
        </p>
      </div>
    </div>
  </div>
</template>

<script>

import { store, mutations } from '../store'
import { eventBus } from '@/eventBus'
import { api } from '@/api'

export default {
  name: 'settings',
  data: function () {
    return {
      services: {
        hostname: '',
        ntp_server_name: '',
        utc_offset_minutes: 0,
        ntp_dst: true,
        mqtt_ip_address: '',
        mqtt_port: '',
        mqtt_user: '',
        mqtt_password: '',
        mqtt_qos: 0,
        enable_ntp_service: false,
        enable_mqtt_service: false,
      },
      leds: [],
      ledOptions: [
        { text: 'Cold White', value: '#DDEFFF' },
        { text: 'Warm White', value: '#FFFDDD' },
        { text: 'Day White', value: '#EAEAEA' },
        { text: 'UV', value: '#8A7AD4' },
        { text: 'Deep Blue', value: '#7C9CFF' },
        { text: 'Blue', value: '#42B8F3' },
        { text: 'Cyan', value: '#4DF7FF' },
        { text: 'Emerald', value: '#4DFFC5' },
        { text: 'Green', value: '#6EB96E' },
        { text: 'Yellow', value: '#FDFE90' },
        { text: 'Amber', value: '#FCBB51' },
        { text: 'Red', value: '#FB647A' },
        { text: 'Deep Red', value: '#990000' }
      ]
    }
  },

  methods: {
    loadSettings () {
      if (store.settings.leds)
        this.leds = [...store.settings.leds]
      if (store.settings.services)
        this.services = store.settings.services
    },

    saveSettings () {
      mutations.setSettings({ leds: this.leds, services: this.services })
      api.setSettings()
    },
  },

  mounted () {
    eventBus.$on('settingsLoaded', () => {
      this.loadSettings()
    })
    eventBus.$on('ledsLoaded', () => {
      this.loadSettings()
    })
    eventBus.$on('servicesLoaded', () => {
      this.loadSettings()
    })
  },

  destroyed() {
    eventBus.$off('settingsLoaded')
    eventBus.$off('ledsLoaded')
    eventBus.$off('servicesLoaded')
  }
}
</script>
