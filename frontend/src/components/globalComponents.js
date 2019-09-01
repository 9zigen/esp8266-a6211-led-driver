import Chart from './Ui/Chart'
import ScheduleChart from '@/components/Ui/ScheduleChart'
import ToggleSwitch from '@/components/Inputs/ToggleSwitch'
import Slider from '@/components/Inputs/InputSlider'

export default {
  install (Vue, options) {
    Vue.component('vue-chart', Chart)
    Vue.component('toggle-switch', ToggleSwitch)
    Vue.component('slider', Slider)
    Vue.component('schedule-chart', ScheduleChart)
  }
}