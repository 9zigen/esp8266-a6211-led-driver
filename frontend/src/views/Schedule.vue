<template>
  <div class="container">
    <h1 class="title is-5">Schedule Settings</h1>
    <h2 class="subtitle is-7"></h2>
    <div class="columns is-centered">
      <div class="column is-10-mobile is-offset-1-mobile is-8">
        <div class="columns is-vcentered">
          <div class="column is-12">
            <vue-chart
                id="schedule-chart"
                type="bar"
                :labels="labels"
                :height="650"
                :colors="colors"
                :lineOptions="{regionFill: 1}"
                :data-sets="series"
            ></vue-chart>
          </div>
        </div>
        <div class="columns is-vcentered">
          <div class="column is-12">
            <div class="columns is-vcentered is-mobile is-multiline" v-for="(point, index) in schedule">
              <div class="column is-3-mobile is-2-tablet">
                <div class="field">
                  <div class="control">
                    <label v-show="index === 0" class="label is-hidden-mobile">Enable:</label>
                    <label class="label is-hidden-tablet">Enable:</label>
                    <toggle-switch v-model="point.enabled"></toggle-switch>
                  </div>
                </div>
              </div>
              <div class="column is-6-mobile is-3-tablet">
                <div class="field is-horizontals">

                  <div class="field-body">
                    <div class="field">
                      <div class="control">
                        <label v-show="index === 0" class="label is-hidden-mobile">Time HH:MM</label>
                        <label class="label is-hidden-tablet">Time HH:MM</label>
                        <input class="input" type="time" placeholder="00" :value="timeToString(point.time_hour, point.time_minute)" v-on:blur="updateTime(index, $event.target.value)">
                      </div>
                    </div>
                  </div>
                </div>
              </div>

              <div class="column is-1 is-visible-mobile is-hidden-tablet">
                <div class="field">
                  <div class="control has-text-centered">
                    <label v-show="index === 0" class="label is-hidden-mobile">Delete</label>
                    <label class="label is-hidden-desktop">Delete</label>
                    <button v-on:click="deleteTimePoint(index)" class="delete is-danger action"></button>
                  </div>
                </div>
              </div>

              <div class="column is-9-mobile" v-for="(ledDuty, ledCh) in point.duty">
                <div class="field is-horizontal">
                  <div class="control">
                    <label v-show="index === 0" class="label is-hidden-mobile">CH {{ledCh + 1}} <span class="is-hidden-tablet">PWM</span></label>
                    <label class="label is-hidden-tablet">CH {{ledCh + 1}} <span class="is-hidden-tablet">PWM</span></label>
                    <input class="input is-primary" type="text" placeholder="PWM Value" v-model.number="point.duty[ledCh]" v-on:blur="updateSeries">
                  </div>
                </div>
              </div>

              <div class="column is-1 is-hidden-mobile">
                <div class="field">
                  <div class="control has-text-centered">
                    <label v-show="index === 0" class="label is-hidden-mobile">Delete</label>
                    <label class="label is-hidden-tablet">Delete</label>
                    <span class="button is-danger" v-on:click="deleteTimePoint(index)"><x-icon></x-icon></span>
                  </div>
                </div>
              </div>
            </div>
          </div>
        </div>

        <div class="columns is-centered">
          <div class="column is-8">

            <div class="buttons is-centered">
              <span class="button is-light" v-on:click="clearTimePoints">Clear</span>
              <span class="button is-light" v-on:click="addTimePoint">Add</span>
              <span class="button is-primary" v-on:click="saveSchedule">Save and continue</span>
              <span class="button is-danger" v-on:click="reloadSchedule">Cancel</span>
            </div>

          </div>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
  import { XIcon } from 'vue-feather-icons'
  import { store, mutations } from '../store'
  import { eventBus } from '../eventBus'
  import { api } from '../api'

  export default {
    name: 'schedule',
    components: {
      XIcon
    },
    data: function () {
      return {
        colors: ['purple', '#ffa3ef', 'light-blue'],
        labels: [],
        series: [],
        schedule: [],
        capacity: 0
      }
    },
    methods: {
      timeFormat (value) {
        let hour = parseInt(value / 60)
        let minutes = value % 60
        return { hour, minutes }
      },
      asTime (value) {
        let hours = parseInt(value / 60)
        let minutes = value % 60
        if (hours < 10) {
          hours = '0' + hours
        }
        if (minutes < 10) {
          minutes = '0' + minutes
        }
        return hours + ':' + minutes
      },
      updateSeries () {
        if (this.schedule.length > 0) {
          let _labels = this.schedule.map((v) => { return this.timeToString(v.time_hour, v.time_minute) })
          let _series = store.settings.leds.map((value, index) => {
            return {
              name: 'LED CH ' + (index + 1),
              values: this.schedule.map((v, i) => {
                return v.duty[index]
              })
            }
          })

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

          // console.log(match) // → {hours: "10", minutes: "01"}
          // console.log(match.groups.hours) // → 10
          // console.log(match.groups.minutes) // → 01

          if (match.groups.hours && match.groups.minutes) {
            let hours = parseInt(match.groups.hours)
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
          let duty = store.settings.leds.map((value, index) => {
            return value.default_duty
          })

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
      clearTimePoints () {
        this.schedule = []
        this.updateSeries()
      },
      saveSchedule () {
        mutations.setSettings({ schedule: { items: this.schedule } })
        api.setSchedule()
      },
      timeToString (hour, minute) {
        let _hour = hour < 10 ? '0' + hour : hour
        let _minute = minute < 10 ? '0' + minute : minute
        return _hour + ':' + _minute
      },
      toMinutes (hour, minute) {
        return hour * 60 + minute
      },

      loadSchedule () {
        /* set colors */
        if (store.settings.leds) {
          this.colors = store.settings.leds.map((value, index, array) => { return value.color })
        }

        /* set data */
        if (store.settings.schedule.items.length > 0) {
          console.log('loadSchedule')

          let _labels = store.settings.schedule.items.map((v) => { return this.timeToString(v.time_hour, v.time_minute) })
          let _series = store.settings.leds.map((value, index) => {
            return {
              name: 'LED CH ' + index,
              values: store.settings.schedule.items.map((v, i) => {
                return v.duty[index]
              })
            }
          })

          this.labels = _labels
          this.series = _series
          this.schedule = [...store.settings.schedule.items]
        }

        /* max capacity */
        if (store.settings.schedule.capacity > 0)
          this.capacity = store.settings.schedule.capacity

      },

      reloadSchedule () {
        api.getSchedule()
      }
    },
    mounted () {
      eventBus.$on('scheduleLoaded', () => {
        this.loadSchedule()
      })
    },
    destroyed() {
      eventBus.$off('scheduleLoaded')
    }
  }
</script>

<style scoped>
  .action {
    margin: 8px!important;
  }
</style>
