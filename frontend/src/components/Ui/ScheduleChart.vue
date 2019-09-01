<template>
  <div>
    <vue-chart
        id="home-chart"
        type="bar"
        :labels="labels"
        :height="650"
        :colors="colors"
        :line-options="{regionFill: 1}"
        :data-sets="series"
    ></vue-chart>
  </div>

</template>

<script>

import { store } from '@/store'
import { eventBus } from '@/eventBus'
import { api } from '@/api'

export default {
  name: "ScheduleChart",
  data: function () {
    return {
      colors: ['purple', '#ffa3ef', 'light-blue'],
      labels: [],
      series: [],
      schedule: []
    }
  },

  methods: {
    timeToString (hour, minute) {
      let _hour = hour < 10 ? '0' + hour : hour
      let _minute = minute < 10 ? '0' + minute : minute
      return _hour + ':' + _minute
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
            name: 'LED CH ' + (index + 1),
            values: store.settings.schedule.items.map((v, i) => {
              return v.duty[index]
            })
          }
        })

        this.labels = _labels
        this.series = _series
        this.schedule = [...store.settings.schedule.items]

      }

    },
  },
  mounted() {
    eventBus.$once('scheduleLoaded', () => {
      this.loadSchedule()
    })
    api.getSchedule()
  }

}
</script>