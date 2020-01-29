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

import { eventBus } from '@/eventBus'
import { http } from '../../http'

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
    this.requestData()
  },
  methods: {
    async requestData () {
      eventBus.$emit('loading', true)
      try {
        let responseLeds = await http.get('/config/leds')
        this.leds = responseLeds.data.leds
        let responseSchedule = await http.get('/config/schedule')
        this.schedule = responseSchedule.data.schedule
        this.drawSchedule()
      } catch (e) {
        eventBus.$emit('message', e, 'danger')
      }
      eventBus.$emit('loading', false)
    },
    timeToString (hour, minute) {
      const _hour = hour < 10 ? `0${hour}` : hour
      const _minute = minute < 10 ? `0${minute}` : minute
      return `${_hour}:${_minute}`
    },
    async drawSchedule () {
      /* set colors */
      if (this.leds) {
        this.colors = this.leds.map((value, index, array) => value.color)
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
    }
  }

}
</script>
