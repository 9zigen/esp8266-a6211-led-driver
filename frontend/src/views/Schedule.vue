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
              <!-- Time -->
              <div class="column is-6-mobile is-2-tablet is-2-desktop">
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
              <div class="column is-6-mobile is-2-tablet">
                <div class="field">
                  <div class="control has-text-centered">
                    <label
                      v-show="index === 0"
                      class="label is-hidden-mobile"
                    >Brightness</label>
                    <label class="label is-hidden-tablet">Brightness %</label>
                    <input
                      v-model.number="point.brightness"
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
                    ><span class="is-hidden-tablet-only">CH </span>{{ ledCh + 1 }}</label>
                    <label class="label is-hidden-tablet">CH {{ ledCh + 1 }}</label>
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

              <!-- Delete Mobile -->
              <div class="column is-12 is-hidden-tablet">
                <div class="field has-addons has-addons-centered">
                  <p class="control">
                    <span
                      class="button is-primary"
                      @click="applyTimePoint(index)"
                    ><check-icon /> Apply Now</span>
                  </p>

                  <p class="control">
                    <span
                      class="button is-danger"
                      @click="deleteTimePoint(index)"
                    ><x-icon /> Delete</span>
                  </p>
                </div>
              </div>

              <!-- Delete Desktop -->
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

              <!-- Apply Desktop -->
              <div class="column is-1 is-hidden-mobile">
                <div class="field">
                  <div class="control has-text-centered">
                    <label
                      v-show="index === 0"
                      class="label is-hidden-mobile"
                    >Now</label>
                    <label class="label is-hidden-tablet">Set</label>
                    <span
                      class="button is-primary"
                      @click="applyTimePoint(index)"
                    ><check-icon /></span>
                  </div>
                  <p class="help is-hidden-mobile">
                    apply now
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
import { eventBus } from '../eventBus'
import { http } from '../http'

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
      leds: [],
      capacity: 5
    }
  },
  mounted () {
    (async () => {
      /* Light schedule */
      let responseSchedule = await http.get('/config/schedule')
      this.schedule = responseSchedule.data.schedule

      /* Led color data */
      let responseLeds = await http.get('/config/leds')
      this.leds = responseLeds.data.leds

      /* Update chart */
      this.loadSchedule()
      this.isLoading = false
    })()
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
        const _series = this.leds.map((value, index) => ({
          name: `LED CH ${index + 1}`,
          values: this.schedule.map((v) => v.duty[index] * v.brightness / 100)
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
        const duty = this.leds.map((value) => 0)

        this.schedule.push({
          enabled: true,
          time_hour: 0,
          time_minute: 0,
          brightness: 100,
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
    async applyTimePoint (id) {
      if (this.schedule[id]) {
        let config = this.schedule[id]
        await http.post('/set/brightness', { brightness: config.brightness })
        await http.post('/set/duty', { duty: config.duty })
        eventBus.$emit('message', 'Applied', 'success')
      }
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
      if (this.leds.length > 0) {
        this.colors = this.leds.map((value) => value.color)
      }

      /* set data */
      if (this.schedule.length > 0) {
        /* Sort array by TIME */
        this.schedule.sort((a, b) => (a.time_hour * 60 + a.time_minute) - (b.time_hour * 60 + b.time_minute))

        const _labels = this.schedule.map(v => this.timeToString(v.time_hour, v.time_minute))
        const _series = this.leds.map((value, index) => ({
          name: `LED CH ${index + 1}`,
          values: this.schedule.map((v) => v.duty[index] * v.brightness / 100)
        }))

        this.labels = _labels
        this.series = _series
      }

      /* max capacity */
      if (this.schedule.capacity > 0) { this.capacity = this.schedule.capacity }

      /* Hide loader */
      eventBus.$emit('loading', false)
    },
    async reloadSchedule () {
      let response = await http.get('/config/schedule')
      this.schedule = response.data.schedule
    },
    async saveSchedule () {
      let schedule = await http.post('/config/schedule', { schedule: this.schedule })
      if (schedule.data.save) {
        eventBus.$emit('message', 'Saved', 'success')
      } else {
        eventBus.$emit('message', 'NOT Saved', 'danger')
      }
    }
  }
}
</script>
