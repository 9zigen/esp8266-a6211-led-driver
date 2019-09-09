<template>
  <div class="container">
    <div class="columns is-marginless">
      <div class="column">
        <div class="content has-text-left">
          <h1 class="title">
            Schedule Settings
          </h1>
        </div>
      </div>
    </div>

    <!-- Chart -->
    <div class="columns is-centered is-marginless">
      <div class="column">
        <div class="notification bg-notification is-light">
          <vue-chart
            id="schedule-chart"
            type="line"
            v-bind:labels="labels"
            v-bind:height="650"
            v-bind:colors="colors"
            v-bind:line-options="{regionFill: 1}"
            v-bind:data-sets="series"
            v-bind:tooltip-options="tooltipOptions"
          />
        </div>
      </div>
    </div>

    <transition-group
      name="list"
      appear
    >
      <!-- Schedule -->
      <div
        v-for="(point, index) in schedule"
        v-bind:key="index"
        class="columns is-marginless"
      >
        <div class="column">
          <div class="notification bg-notification is-light">
            <div class="columns is-mobile is-multiline">
              <!-- Enable -->
              <div class="column is-4-mobile is-2-tablet">
                <div class="field">
                  <div class="control has-text-centered">
                    <label
                      v-show="index === 0"
                      class="label is-hidden-mobile"
                    >Enable:</label>
                    <label class="label is-hidden-tablet">Enable:</label>
                    <toggle-switch
                      v-model="point.enabled"
                      round
                    />
                  </div>
                  <p class="help is-hidden-mobile">
                    ON/OFF
                  </p>
                </div>
              </div>

              <!-- Delete Mobile -->
              <div class="column is-8 is-hidden-tablet">
                <div class="field">
                  <div class="control has-text-centered">
                    <label
                      v-show="index === 0"
                      class="label is-hidden-mobile"
                    >Action</label>
                    <label class="label is-hidden-tablet">Action</label>
                    <div class="buttons has-addons is-centered">
                      <span
                        class="button is-primary"
                        @click="applyTimePoint(index)"
                      ><check-icon /> Apply Now</span>
                      <span
                        class="button is-danger"
                        @click="deleteTimePoint(index)"
                      ><x-icon /> Delete</span>
                    </div>
                  </div>
                  <p class="help is-hidden-mobile">
                    schedule
                  </p>
                </div>
              </div>

              <!-- Time -->
              <div class="column is-12-mobile is-2-tablet is-2-desktop">
                <div class="field">
                  <div class="control has-text-centered">
                    <label
                      v-show="index === 0"
                      class="label is-hidden-mobile"
                    >HH:MM</label>
                    <label class="label is-hidden-tablet">HH:MM</label>
                    <input
                      class="input"
                      type="time"
                      placeholder="00"
                      v-bind:value="timeToString(point.time_hour, point.time_minute)"
                      @blur="updateTime(index, $event.target.value)"
                    >
                  </div>
                  <p class="help is-hidden-mobile">
                    Time to apply
                  </p>
                </div>
              </div>

              <!-- Bright -->
              <div class="column is-12-mobile is-2-tablet">
                <div class="field">
                  <div class="control has-text-centered">
                    <label
                      v-show="index === 0"
                      class="label is-hidden-mobile"
                    >Brightness</label>
                    <label class="label is-hidden-tablet">Brightness</label>
                    <input
                      v-model.number="point.led_brightness"
                      class="input is-primary"
                      type="text"
                      placeholder="Brightness"
                      @blur="updateSeries"
                    >
                  </div>
                  <p class="help is-hidden-mobile">
                    0-255
                  </p>
                </div>
              </div>

              <!-- Led channels -->
              <div
                v-for="(ledDuty, ledCh) in point.duty"
                v-bind:key="ledCh"
                class="column is-4-mobile"
              >
                <div class="field">
                  <div class="control has-text-centered">
                    <label
                      v-show="index === 0"
                      class="label is-hidden-mobile"
                    ><span class="is-hidden-tablet-only">CH </span>{{ ledCh }}</label>
                    <label class="label is-hidden-tablet">CH {{ ledCh }}</label>
                    <input
                      v-model.number="point.duty[ledCh]"
                      class="input is-primary"
                      type="text"
                      placeholder="PWM Value"
                      @blur="updateSeries"
                    >
                  </div>
                  <p class="help">
                    0-255
                  </p>
                </div>
              </div>

              <div class="column is-1 is-hidden-mobile">
                <div class="field">
                  <div class="control has-text-centered">
                    <label
                      v-show="index === 0"
                      class="label is-hidden-mobile"
                    >Delete</label>
                    <label class="label is-hidden-tablet">Delete</label>
                    <span
                      class="button is-danger"
                      @click="deleteTimePoint(index)"
                    ><x-icon /></span>
                  </div>
                  <p class="help is-hidden-mobile">
                    schedule
                  </p>
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
        class="button is-light"
        @click="addTimePoint"
      ><plus-icon />Add</span>
      <span
        class="button is-primary"
        @click="saveSchedule"
      ><check-icon /> Save and continue</span>
      <span
        class="button is-danger"
        @click="reloadSchedule"
      ><x-icon /> Cancel</span>
    </div>
  </div>
</template>

<script>
import { store, mutations } from '../store'
import { eventBus } from '../eventBus'
import { api } from '../api'

export default {
  name: 'Schedule',
  data () {
    return {
      colors: ['purple', '#ffa3ef', 'light-blue'],
      tooltipOptions: {
        formatTooltipX: d => (`${d}`).toUpperCase(),
        formatTooltipY: d => `${d} PWM (${parseInt(d / 255 * 100)} %)`
      },
      labels: [],
      series: [],
      schedule: [],
      capacity: 0
    }
  },
  mounted () {
    eventBus.$on('scheduleLoaded', () => {
      this.loadSchedule()
    })
  },
  destroyed () {
    eventBus.$off('scheduleLoaded')
  },
  methods: {
    timeFormat (value) {
      const hour = parseInt(value / 60)
      const minutes = value % 60
      return { hour, minutes }
    },
    asTime (value) {
      let hours = parseInt(value / 60)
      let minutes = value % 60
      if (hours < 10) {
        hours = `0${hours}`
      }
      if (minutes < 10) {
        minutes = `0${minutes}`
      }
      return `${hours}:${minutes}`
    },
    updateSeries () {
      if (this.schedule.length > 0) {
        const _labels = this.schedule.map(v => this.timeToString(v.time_hour, v.time_minute))
        const _series = store.settings.leds.map((value, index) => ({
          name: `LED CH ${index + 1}`,
          values: this.schedule.map((v) => v.duty[index] * v.led_brightness / 255)
        }))

        this.labels = _labels
        this.series = _series
      } else {
        this.series = []
      }
    },
    updateTime (index, value) {
      if (value !== '') {
        const re = /(?<hours>\d{2}):(?<minutes>\d{2})/
        const match = re.exec(value)

        if (match.groups.hours && match.groups.minutes) {
          const hours = parseInt(match.groups.hours)
          if (hours > 24) {
            match.groups.hours = 24
          }

          let minutes = parseInt(match.groups.minutes)
          if (minutes > 60) {
            minutes = 60
          }

          this.schedule[index].time_hour = hours
          this.schedule[index].time_minute = minutes
        }
      }
    },
    addTimePoint () {
      if (this.schedule.length < this.capacity) {
        /* use default duty from settings */
        const duty = store.settings.leds.map((value) => value.default_duty)

        this.schedule.push({
          enabled: true,
          time_hour: 0,
          time_minute: 0,
          duty: [...duty]
        })
        this.updateSeries()
      } else {
        /* error - MAX Schedules reached */
        eventBus.$emit('message', 'MAX Schedules reached', 'danger')
      }
    },
    deleteTimePoint (id) {
      this.schedule.splice(id, 1)
      this.updateSeries()
    },
    applyTimePoint (id) {
      if (this.schedule[id]) {
        let config = this.schedule[id]
        api.setBrightness(config.led_brightness)
        api.setDuty(config.duty)
      }
    },
    saveSchedule () {
      mutations.setSettings({ schedule: { items: this.schedule } })
      api.setSchedule()
    },
    timeToString (hour, minute) {
      const _hour = hour < 10 ? `0${hour}` : hour
      const _minute = minute < 10 ? `0${minute}` : minute
      return `${_hour}:${_minute}`
    },
    toMinutes (hour, minute) {
      return hour * 60 + minute
    },

    loadSchedule () {
      /* set colors */
      if (store.settings.leds) {
        this.colors = store.settings.leds.map((value) => value.color)
      }

      /* set data */
      if (store.settings.schedule.items.length > 0) {
        /* Sort array by TIME */
        store.settings.schedule.items.sort((a, b) => (a.time_hour * 60 + a.time_minute) - (b.time_hour * 60 + b.time_minute))

        const _labels = store.settings.schedule.items.map(v => this.timeToString(v.time_hour, v.time_minute))
        const _series = store.settings.leds.map((value, index) => ({
          name: `LED CH ${index}`,
          values: store.settings.schedule.items.map((v) => v.duty[index] * v.led_brightness / 255)
        }))

        this.labels = _labels
        this.series = _series
        this.schedule = [...store.settings.schedule.items]
      }

      /* max capacity */
      if (store.settings.schedule.capacity > 0) { this.capacity = store.settings.schedule.capacity }

      /* Hide loader */
      eventBus.$emit('loading', false)
    },

    reloadSchedule () {
      api.getSchedule()
    }
  }
}
</script>
