<template>
  <div>
    <apex-chart width="100%" type="area" :options="options" :series="series"></apex-chart>
  </div>
  
</template>

<script>

import ApexChart from 'vue-apexcharts'
import { store } from '@/store'
import EventBus from '@/eventBus'

export default {
  name: "ScheduleChart",
  components: {
    ApexChart
  },
  data: function () {
    return {
      options: {
        chart: {
          id: 'chart-light'
        },
        legend: {
          position: 'top'
        },
        plotOptions: {
          bar: {
            horizontal: false,
            columnWidth: '55%',
            // endingShape: 'rounded'
          }
        },
        colors: [ '#F44336', '#E91E63', '#9C27B0' ],
        fill: {
          type: 'gradient',
          gradient: {
            opacityFrom: 0.6,
            opacityTo: 0.8
          }
        },
        dataLabels: {
          enabled: false
        },
        stroke: {
          show: true,
          width: 2,
          curve: 'straight'
          // colors: ['transparent']
        },
        yaxis: {
          labels: {
            formatter: function (val) {
              return val
            }
          }
        }
      },
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
      if (store.settings.schedule.items.length > 0) {
        let _series = store.settings.leds.map((value, index) => {
          return {
            name: 'LED CH ' + index,
            data: store.settings.schedule.items.map((v, i) => {
              return {
                x: this.timeToString(v.time_hour, v.time_minute),
                y: v.duty[index]
              }
            })
          }
        })

        this.series = [..._series]
        this.schedule = [...store.settings.schedule.items]
      }

      if (store.settings.leds) {
        const colors = store.settings.leds.map((value, index, array) => { return value.color })
        this.options = Object.assign({}, this.options, { colors: colors })
      }

    },
  },
  mounted() {
    EventBus.$on('scheduleLoaded', id => {
      this.loadSchedule()
    })
    EventBus.$emit('getSchedule')
  }

}
</script>

<style scoped>

</style>