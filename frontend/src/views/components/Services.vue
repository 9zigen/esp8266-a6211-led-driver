<template>
  <div class="columns is-marginless is-multiline">
    <div class="column is-12">
      <!-- MAIN -->
      <div class="notification bg-notification is-light">
        <div class="field is-horizontal">
          <div class="field-label is-normal">
            <label class="label">Identity</label>
          </div>
          <div class="field-body">
            <div class="field">
              <div class="control">
                <input
                  v-model="services.hostname"
                  class="input"
                  type="text"
                  placeholder="Hostname"
                >
              </div>
            </div>
          </div>
        </div>
      </div>

      <!-- NTP -->
      <div class="notification bg-notification is-light">
        <div class="field is-horizontal">
          <div class="field-label is-normal">
            <label class="label">NTP</label>
          </div>
          <div class="field-body">
            <div class="field">
              <div class="control has-text-centered">
                <toggle-switch
                  v-model="services.enable_ntp_service"
                  round
                />
              </div>
              <p class="help">
                Enable
              </p>
            </div>
            <div class="field">
              <div class="control has-text-centered">
                <toggle-switch
                  v-model="services.ntp_dst"
                  round
                />
              </div>
              <p class="help">
                Daylight saving time
              </p>
            </div>
            <div class="field">
              <div class="control">
                <input
                  v-model="services.ntp_server_name"
                  class="input"
                  type="text"
                  placeholder="NTP Server Name"
                >
              </div>
              <p class="help">
                Server Name
              </p>
            </div>
            <div class="field">
              <div class="control">
                <input
                  v-model="services.utc_offset_minutes"
                  class="input"
                  type="text"
                  placeholder="UTC Offset"
                >
              </div>
              <p class="help">
                UTC Offset
              </p>
            </div>
          </div>
        </div>
      </div>

      <!-- MQTT -->
      <div class="notification bg-notification is-light">
        <div class="field is-horizontal">
          <div class="field-label is-normal">
            <label class="label">MQTT</label>
          </div>
          <div class="field-body">
            <div class="field">
              <div class="control has-text-centered">
                <toggle-switch
                  v-model="services.enable_mqtt_service"
                  round
                />
              </div>
              <p class="help">
                Enable
              </p>
            </div>
            <div class="field">
              <div class="control">
                <div class="select">
                  <select
                    v-model="services.mqtt_qos"
                    name="mqtt_qos"
                  >
                    <option
                      v-for="(option, index) in mqttOptions"
                      v-bind:key="index"
                      v-bind:value="option.value"
                    >
                      {{ option.text }}
                    </option>
                  </select>
                </div>
              </div>
              <p class="help">
                MQTT QoS
              </p>
            </div>
            <div class="field">
              <div class="control">
                <input
                  v-model="services.mqtt_ip_address"
                  v-mask="'###.###.###.###'"
                  class="input"
                  type="text"
                  placeholder="MQTT IP Address"
                >
              </div>
              <p class="help">
                MQTT Server IP Address
              </p>
            </div>
            <div class="field">
              <div class="control">
                <input
                  v-model="services.mqtt_port"
                  class="input"
                  type="text"
                  placeholder="MQTT Server Port"
                >
              </div>
              <p class="help">
                MQTT Server Port
              </p>
            </div>
          </div>
        </div>
        <div class="field is-horizontal">
          <div class="field-label is-normal">
            <label class="label" />
          </div>
          <div class="field-body">
            <div class="field">
              <div class="control">
                <input
                  v-model="services.mqtt_user"
                  class="input"
                  type="text"
                  placeholder="User"
                >
              </div>
              <p class="help">
                MQTT User
              </p>
            </div>
            <div class="field">
              <div class="control">
                <input
                  v-model="services.mqtt_password"
                  class="input"
                  type="text"
                  placeholder="Password"
                >
              </div>
              <p class="help">
                MQTT Password
              </p>
            </div>
          </div>
        </div>
      </div>
    </div>

    <div class="column">
      <!-- Buttons -->
      <div class="buttons is-centered">
        <span
          class="button is-primary"
          @click="saveServices"
        >
          <check-icon
            size="1.5x"
            class="custom-class"
          /> Apply
        </span>
        <span
          class="button is-danger"
          @click="loadSettings"
        >
          <x-icon
            size="1.5x"
            class="custom-class"
          /> Cancel
        </span>
      </div>
    </div>
  </div>
</template>

<script>
import { store, mutations } from '@/store'
import { eventBus } from '@/eventBus'
import { api } from '@/api'

export default {
  name: 'Services',
  data () {
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
        enable_mqtt_service: false
      },
      mqttOptions: [
        { text: 'At most once (0)', value: '0' },
        { text: 'At least once (1)', value: '1' },
        { text: 'Exactly once (2)', value: '2' }
      ]
    }
  },
  mounted () {
    eventBus.$on('servicesLoaded', () => {
      this.loadServices()
    })
    this.loadServices()
  },
  destroyed () {
    eventBus.$off('servicesLoaded')
  },
  methods: {
    loadServices () {
      if (store.settings.services) {
        this.services = store.settings.services
      } else {
        api.getSettings()
      }
    },
    saveServices () {
      mutations.setSettings({ services: this.services })
      api.setServices()
    },
    loadSettings () {
      api.getSettings()
    }
  }
}
</script>

<style scoped>

</style>
