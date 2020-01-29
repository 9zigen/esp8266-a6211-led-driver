<template>
  <div v-bind:id="id" />
</template>

<script>
import { Chart } from 'frappe-charts/dist/frappe-charts.min.esm'

export default {
  name: 'Chart',
  props: {
    id: {
      required: true,
      type: String,
      default: null
    },

    dataSets: {
      required: false,
      type: Array,
      default: () => []
    },

    labels: {
      required: false,
      type: Array,
      default: () => []
    },

    startDate: {
      required: false,
      type: Date,
      default: null
    },

    endDate: {
      required: false,
      type: Date,
      default: null
    },

    dataPoints: {
      required: false,
      type: Object,
      default: () => {}
    },

    countLabel: {
      required: false,
      type: String,
      default: 'Count'
    },

    title: {
      required: false,
      type: String,
      default: () => ''
    },

    height: {
      required: false,
      type: Number,
      default: 300
    },

    type: {
      required: false,
      type: String,
      default: 'bar'
    },

    yMarkers: {
      required: false,
      type: Array,
      default: () => [
        {
          label: 'Marker',
          value: 70,
          options: {
            labelPos: 'left'
          }
        }
      ]
    },

    yRegions: {
      required: false,
      type: Array,
      default: () => [
        {
          label: 'Region',
          start: -10,
          end: 50,
          options: {
            labelPos: 'right'
          }
        }
      ]
    },

    colors: {
      required: false,
      type: Array,
      default: () => [
        'purple', '#ffa3ef', 'light-blue'
      ]
    },

    isNavigable: {
      required: false,
      type: Boolean,
      default: false
    },

    valuesOverPoints: {
      required: false,
      type: Boolean,
      default: false
    },

    lineOptions: {
      required: false,
      type: Object,
      default: () => ({
        dotSize: 4,
        hideLine: 0,
        hideDots: 0,
        heatline: 0,
        regionFill: 0,
        areaFill: 0
      })
    },

    axisOptions: {
      required: false,
      type: Object,
      default: () => ({
        yAxisMode: '',
        xAxisMode: '',
        xIsSeries: 0
      })
    },

    maxLegendPoints: {
      required: false,
      type: Number,
      default: 20
    },

    maxSlices: {
      required: false,
      type: Number,
      default: 20
    },

    barOptions: {
      required: false,
      type: Object,
      default: () => ({
        height: 20,
        depth: 2,
        spaceRatio: 0.5,
        stacked: 0
      })
    },

    discreteDomains: {
      required: false,
      type: Boolean,
      default: true
    },

    tooltipOptions: {
      required: false,
      type: Object,
      default: () => ({
        formatTooltipX: d => (`${d}`).toUpperCase(),
        formatTooltipY: d => `${d} pts`
      })
    }
  },

  data () {
    return {
      chart: null,
      data: {
        labels: this.labels,
        datasets: this.dataSets
      },

      heatmapData: {
        dataPoints: this.dataPoints,
        start: this.startDate,
        end: this.endDate,
        countLabel: this.countLabel
      },

      firstRun: true
    }
  },

  watch: {
    labels: {
      handler (val) {
        this.$set(this.data, 'labels', val)
      },
      deep: true
    },
    dataSets: {
      handler (val) {
        this.$set(this.data, 'datasets', val)
        if (this.firstRun) {
          this.firstRun = false
          this.startChart()
        } else {
          this.update(this.data)
        }
      },
      deep: true
    }
  },

  mounted () {
    // this.startChart()
  },

  methods: {
    startChart () {
      const baseOptions = {
        type: this.type,
        discreteDomains: this.discreteDomains,
        colors: this.colors,
        height: this.height,
        title: this.title,
        isNavigable: this.isNavigable
      }

      const heatMapOptions = {
        data: this.heatmapData
      }

      const chartOptions = {
        data: this.data,
        tooltipOptions: this.tooltipOptions,
        valuesOverPoints: this.valuesOverPoints,
        barOptions: this.barOptions,
        lineOptions: this.lineOptions,
        axisOptions: this.axisOptions,
        maxLegendPoints: this.maxLegendPoints,
        maxSlices: this.maxSlices
      }

      const options = Object.assign(
        baseOptions,
        (this.type === 'heatmap') ? heatMapOptions : chartOptions
      )

      this.chart = new Chart(`#${this.id}`, options)
    },

    export () {
      this.chart.export()
    },

    addDataPoint (label, valueFromEachDataset, index) {
      this.chart.addDataPoint(label, valueFromEachDataset, index)
    },

    removeDataPoint (index) {
      this.chart.removeDataPoint(index)
    },

    updateDataset (datasetValues, index) {
      this.chart.updateDataset(datasetValues, index)
    },

    update (data) {
      this.chart.update(data)
    },

    unbindWindowEvents () {
      this.chart.unbindWindowEvents()
    }
  }
}
</script>
