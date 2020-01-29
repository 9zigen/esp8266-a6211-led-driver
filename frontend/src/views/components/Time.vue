<template>
  <div class="container">
    <div class="tile is-ancestor is-marginless">
      <div class="tile is-vertical">
        <!-- top row -->
        <div class="tile">
          <!-- left col -->
          <div class="tile is-vertical is-4">
            <div class="tile is-parent">
              <article class="tile is-child notification bg-notification is-light">
                <div class="content">
                  <p class="subtitle">
                    Local Time
                  </p>
                  <div class="content">
                    <clock v-bind:time="timeString" />
                  </div>
                </div>
              </article>
            </div>
          </div>

          <!-- right col -->
          <div class="tile is-vertical">
            <div class="tile is-parent">
              <article class="tile is-child notification bg-notification is-light">
                <h5 class="subtitle">
                  Device Time
                </h5>
                <div class="field is-horizontal">
                  <div class="field-label is-normal">
                    <label class="label">Date</label>
                  </div>
                  <div class="field-body">
                    <div class="field">
                      <div class="control">
                        <input-number
                          v-model="deviceTime.year"
                          class="input"
                          maxlength="2"
                          placeholder="year"
                        />
                      </div>
                      <p class="help">
                        Year
                      </p>
                    </div>
                    <div class="field">
                      <div class="control">
                        <input
                          v-model="deviceTime.month"
                          class="input"
                          type="text"
                          placeholder="month"
                        >
                      </div>
                      <p class="help">
                        Month
                      </p>
                    </div>
                    <div class="field">
                      <div class="control">
                        <input
                          v-model="deviceTime.day"
                          class="input"
                          type="text"
                          placeholder="month"
                        >
                      </div>
                      <p class="help">
                        Day
                      </p>
                    </div>
                    <div class="field">
                      <div class="control">
                        <div class="select">
                          <select
                            v-model="deviceTime.weekday"
                            name="weekday"
                          >
                            <option
                              v-for="(option, index) in weekdayOptions"
                              v-bind:key="index"
                              v-bind:value="option.value"
                            >
                              {{ option.text }}
                            </option>
                          </select>
                        </div>
                      </div>
                      <p class="help">
                        Weekday
                      </p>
                    </div>
                  </div>
                </div>

                <div class="field is-horizontal">
                  <div class="field-label is-normal">
                    <label class="label">Time</label>
                  </div>
                  <div class="field-body">
                    <!-- Hours -->
                    <div class="field">
                      <div class="control">
                        <input
                          v-model="deviceTime.hour"
                          class="input"
                          type="text"
                          placeholder="Hours"
                        >
                      </div>
                      <p class="help">
                        Hours
                      </p>
                    </div>

                    <!-- Minutes -->
                    <div class="field">
                      <div class="control">
                        <input
                          v-model="deviceTime.minute"
                          class="input"
                          type="text"
                          placeholder="Minutes"
                        >
                      </div>
                      <p class="help">
                        Minutes
                      </p>
                    </div>

                    <!-- Seconds -->
                    <div class="field">
                      <div class="control">
                        <input
                          v-model="deviceTime.second"
                          class="input"
                          type="text"
                          placeholder="Minutes"
                        >
                      </div>
                      <p class="help">
                        Seconds
                      </p>
                    </div>

                    <!-- DST -->
                    <div class="field">
                      <div class="control has-text-centered">
                        <toggle-switch
                          v-model="deviceTime.dst"
                          round
                        />
                      </div>
                      <p class="help">
                        Daylight saving time
                      </p>
                    </div>

                    <!-- UTC offset -->
                    <div class="field">
                      <div class="control">
                        <input
                          v-model="deviceTime.utc"
                          class="input"
                          type="text"
                          placeholder="UTC Offset Hours"
                        >
                      </div>
                      <p class="help">
                        UTC Offset (minutes)
                      </p>
                    </div>
                  </div>
                </div>
              </article>
            </div>
          </div>
        </div>

        <!-- bottom row -->
        <div class="tile">
          <div class="tile is-vertical">
            <div class="tile is-parent">
              <article class="tile is-child notification bg-notification is-light">
                <div class="content">
                  <div class="buttons is-centered">
                    <span
                      class="button is-primary"
                      @click="saveTime"
                    >
                      <check-icon
                        size="1.5x"
                        class="custom-class"
                      /> Apply
                    </span>
                    <span
                      class="button is-danger"
                      @click="loadTime"
                    >
                      <x-icon
                        size="1.5x"
                        class="custom-class"
                      /> Cancel
                    </span>
                  </div>
                </div>
              </article>
            </div>
          </div>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
import { eventBus } from '@/eventBus'
import { http } from '@/http'
import Clock from '../../components/Ui/Clock'

export default {
  name: 'Services',
  components: { Clock },
  data () {
    return {
      timeString: '',
      browserTime: {
        hour: '',
        minute: '',
        seconds: ''
      },
      deviceTime: {
        year: 20,
        month: 10,
        weekday: 1,
        day: 10,
        hour: 12,
        minute: 1,
        second: 1,
        dst: 0,
        utc: 1
      },
      dstOptions: [
        { text: 'No ', value: '0' },
        { text: 'Yes', value: '1' }
      ],
      weekdayOptions: [
        { text: 'Monday', value: '1' },
        { text: 'Tuesday', value: '2' },
        { text: 'Wednesday', value: '3' },
        { text: 'Thursday', value: '4' },
        { text: 'Friday', value: '5' },
        { text: 'Saturday', value: '6' },
        { text: 'Sunday', value: '7' }
      ]
    }
  },
  mounted () {
    this.loadTime()

    /* update current time */
    setInterval(() => {
      let dateTime = new Date()
      this.browserTime.hour = dateTime.getHours()
      this.browserTime.minute = dateTime.getMinutes()
      this.browserTime.seconds = dateTime.getSeconds()
      this.timeString = this.timeToString()
    }, 1000)
  },
  methods: {
    async saveTime () {
      let response = await http.post('/time', { time: this.deviceTime })
      this.deviceTime = response.data
      eventBus.$emit('message', 'Saved', 'success')
    },
    async loadTime () {
      let response = await http.get('/time')
      this.deviceTime = response.data
    },
    timeToString () {
      return this.browserTime.hour + ':' + this.browserTime.minute + ':' + this.browserTime.seconds
    }
  }
}
</script>
