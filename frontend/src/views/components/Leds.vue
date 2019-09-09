<template>
  <div>
    <transition-group
      name="list"
      appear
    >
      <div
        v-for="led in leds"
        v-bind:key="led.id"
        class="columns is-marginless"
      >
        <div class="column">
          <div class="notification bg-notification is-light">
            <div class="field is-horizontal">
              <div class="field-label is-normal">
                <label class="label">Channel {{ led.id }}</label>
              </div>
              <div class="field-body">
                <div class="field">
                  <div class="control">
                    <div class="select">
                      <select
                        v-model="led.color"
                        name="color"
                      >
                        <option
                          v-for="(option, index) in $ledOptions"
                          v-bind:key="index"
                          v-bind:value="option.value"
                        >
                          {{ option.text }}
                        </option>
                      </select>
                    </div>
                    <p class="help">
                      Channel Color
                    </p>
                  </div>
                </div>
                <div class="field">
                  <div class="control">
                    <input
                      v-model="led.channel_power"
                      class="input"
                      type="text"
                      placeholder="channel power"
                    >
                  </div>
                  <p class="help">
                    Channel Power (Watts)
                  </p>
                </div>
                <div class="field">
                  <div class="control">
                    <input
                      v-model="led.default_duty"
                      class="input"
                      type="text"
                      placeholder="default duty"
                    >
                  </div>
                  <p class="help">
                    Default Duty (0-255)
                  </p>
                </div>
                <div class="field">
                  <div class="control has-text-centered">
                    <toggle-switch
                      v-model.number="led.state"
                      round
                    />
                  </div>
                  <p class="help">
                    Default State ON/OFF
                  </p>
                </div>
              </div>
            </div>

            <span>
              MQTT Topics
            </span>

            <div class="field is-horizontal">
              <div class="field-label is-normal">
                <label class="label">State ON/OFF</label>
              </div>
              <div class="field-body">
                <div class="field">
                  <div class="control is-text">
                    {{ services.hostname }}/channel/{{ led.id }}/state
                  </div>
                  <span class="help">Get</span>
                </div>
                <div class="field">
                  <div class="control is-text">
                    {{ services.hostname }}/channel/{{ led.id }}/switch
                  </div>
                  <span class="help">Set</span>
                </div>
              </div>
            </div>

            <div class="field is-horizontal">
              <div class="field-label is-normal">
                <label class="label">Brightness</label>
              </div>
              <div class="field-body">
                <div class="field">
                  <div class="control is-text">
                    {{ services.hostname }}/brightness
                  </div>
                  <span class="help">Get</span>
                </div>
                <div class="field">
                  <div class="control is-text">
                    {{ services.hostname }}/brightness/set
                  </div>
                  <span class="help">Set</span>
                </div>
              </div>
            </div>

            <div class="field is-horizontal">
              <div class="field-label is-normal">
                <label class="label">Duty (0 - 255)</label>
              </div>
              <div class="field-body">
                <div class="field">
                  <div class="control is-text">
                    {{ services.hostname }}/channel/{{ led.id }}
                  </div>
                  <span class="help">Get</span>
                </div>
                <div class="field">
                  <div class="control is-text">
                    {{ services.hostname }}/channel/{{ led.id }}/set
                  </div>
                  <span class="help">Set</span>
                </div>
              </div>
            </div>
          </div>
        </div>
      </div>
    </transition-group>

    <!-- Buttons -->
    <div class="buttons is-centered">
      <span
        class="button is-primary"
        @click="saveLeds"
      >
        <check-icon size="1.5x" /> Apply
      </span>

      <span
        class="button is-danger"
        @click="loadSettings"
      >
        <x-icon size="1.5x" /> Cancel
      </span>
    </div>
  </div>
</template>

<script>
import { store, mutations } from '@/store'
import { eventBus } from '@/eventBus'
import { api } from '@/api'

export default {
  name: 'Leds',
  data () {
    return {
      services: {},
      leds: []
    }
  },
  mounted () {
    eventBus.$on('ledsLoaded', () => {
      this.loadLeds()
    })
    this.loadLeds()
  },
  destroyed () {
    eventBus.$off('ledsLoaded')
  },
  methods: {
    loadLeds () {
      if (store.settings.leds && store.settings.services) {
        this.leds = [...store.settings.leds]
        this.services = store.settings.services
      } else {
        api.getSettings()
      }
    },
    saveLeds () {
      mutations.setSettings({ leds: this.leds })
      api.setLeds()
    },
    loadSettings () {
      api.getSettings()
    }
  }
}
</script>

<style scoped>

</style>
