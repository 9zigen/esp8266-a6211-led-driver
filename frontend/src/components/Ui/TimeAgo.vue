<template>
  <span>{{ timeago }} <span class="time-ago-text">{{ nowString }}</span></span>
</template>

<script>
export default {
  name: 'TimeAgo',
  props: {
    timestamp: {
      type: [Number],
      default: e => (Date.now())
    },
    locale: {
      type: String,
      default: 'en'
    },
    refresh: {
      type: [Number, Boolean],
      default: false
    }
  },
  data () {
    return {
      timeago: '',
      nowString: '',
      intervalId: null
    }
  },
  mounted () {
    this.$nextTick(() => {
      this.reloadTime()
      if (this.refresh) {
        // const refreshTime = this.refresh === true ? 60 : this.refresh
        this.intervalId = setInterval(this.reloadTime, this.refresh * 1000)
      }
    })
  },
  destroyed () {
    if (this.intervalId) clearInterval(this.intervalId)
  },
  methods: {
    reloadTime () {
      const { timeago, nowString } = this.calcTimeAgo(this.timestamp, this.locale)
      this.timeago = timeago
      this.nowString = nowString
    },
    calcTimeAgo (timeago, nowString, format) {
      const difference = Date.now() - timeago

      if (difference < 1000) {
        return { timeago: 'now', nowString: '' }
      }

      const sec = Math.floor(difference / 1000)
      if (sec < 60) {
        return { timeago: sec, nowString: 'sec ago' }
      }

      const min = Math.floor(difference / 60000)
      if (min < 60) {
        return { timeago: min, nowString: 'min ago' }
      }

      const hours = Math.floor(difference / 60000 * 24)
      if (hours === 1) {
        return { timeago: hours, nowString: 'hour ago' }
      }
      if (hours < 24) {
        return { timeago: hours, nowString: 'hours ago' }
      }

      const date = new Date(timeago)
      // let str = date.getDate() + '/' + date.getMonth() + '/' + date.getFullYear()
      return { timeago: date.toLocaleDateString(this.locale), nowString: '' }
    }
  }
}
</script>

<style scoped>
  .time-ago-text {
    color: dimgray;
  }
</style>
