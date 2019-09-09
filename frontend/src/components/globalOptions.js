const ledOptions = [
  { text: 'Cold White', value: '#DDEFFF' },
  { text: 'Warm White', value: '#FFFDDD' },
  { text: 'Day White', value: '#EAEAEA' },
  { text: 'UV', value: '#8A7AD4' },
  { text: 'Deep Blue', value: '#7C9CFF' },
  { text: 'Blue', value: '#42B8F3' },
  { text: 'Cyan', value: '#4DF7FF' },
  { text: 'Emerald', value: '#4DFFC5' },
  { text: 'Green', value: '#6EB96E' },
  { text: 'Yellow', value: '#FDFE90' },
  { text: 'Amber', value: '#FCBB51' },
  { text: 'Red', value: '#FB647A' },
  { text: 'Deep Red', value: '#990000' }
]

export default {
  install (Vue, options) {
    Vue.prototype.$appName = 'HV LED Driver App'
    Vue.prototype.$ledOptions = ledOptions
  }
}
