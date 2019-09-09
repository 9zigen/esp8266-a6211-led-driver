<template>
  <div>
    <vue-chart
      id="home-chart"
      type="line"
      v-bind:labels="labels"
      v-bind:height="650"
      v-bind:colors="colors"
      v-bind:line-options="{regionFill: 1}"
      v-bind:data-sets="series"
      v-bind:tooltip-options="tooltipOptions"
    />
  </div>
</template>

<script>

import { store } from '@/store'
import { eventBus } from '@/eventBus'
import { api } from '@/api'

export default {
  name: 'ScheduleChart',
  data () {
    return {
      colors: ['purple', '#ffa3ef', 'light-blue'],
      tooltipOptions: {
        formatTooltipX: d => (`${d}`).toUpperCase(),
        formatTooltipY: d => `${d} PWM (${parseInt(d / 255 * 100)} %)`
      },
      labels: [],
      series: [],
      schedule: []
    }
  },
  mounted () {
    eventBus.$once('scheduleLoaded', () => {
      this.loadSchedule()
    })
    api.getSchedule()
  },

  methods: {
    timeToString (hour, minute) {
      const _hour = hour < 10 ? `0${hour}` : hour
      const _minute = minute < 10 ? `0${minute}` : minute
      return `${_hour}:${_minute}`
    },
    loadSchedule () {
      /* set colors */
      if (store.settings.leds) {
        this.colors = store.settings.leds.map((value, index, array) => value.color)
      }

      /* set data */
      if (store.settings.schedule.items.length > 0) {
        console.log('loadSchedule')

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
    }
  }

}
</script>
