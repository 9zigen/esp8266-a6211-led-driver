import { CheckIcon, XIcon, PlusIcon, ClockIcon, TrelloIcon, SettingsIcon, InfoIcon, WifiIcon } from 'vue-feather-icons'
import Chart from './Ui/Chart'
import ScheduleChart from '@/components/Ui/ScheduleChart'
import ToggleSwitch from '@/components/Inputs/ToggleSwitch'
import Slider from '@/components/Inputs/InputSlider'
import InputIP from '@/components/Inputs/InputIP'
import InputNumber from './Inputs/InputNumber'

export default {
  install (Vue, options) {
    Vue.component('vue-chart', Chart)
    Vue.component('toggle-switch', ToggleSwitch)
    Vue.component('slider', Slider)
    Vue.component('input-ip', InputIP)
    Vue.component('input-number', InputNumber)
    Vue.component('schedule-chart', ScheduleChart)

    /* feather-icons */
    Vue.component('CheckIcon', CheckIcon)
    Vue.component('XIcon', XIcon)
    Vue.component('PlusIcon', PlusIcon)
    Vue.component('ClockIcon', ClockIcon)
    Vue.component('TrelloIcon', TrelloIcon)
    Vue.component('SettingsIcon', SettingsIcon)
    Vue.component('InfoIcon', InfoIcon)
    Vue.component('WifiIcon', WifiIcon)
  }
}
