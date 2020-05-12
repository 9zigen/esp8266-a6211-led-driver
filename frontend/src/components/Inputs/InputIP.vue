<template>
  <div class="input-wrapper">
    <div
      v-for="(segment, index) in ipCopy"
      v-bind:key="index"
      class="segment"
    >
      <input
        ref="ipSegment"
        v-model="ipCopy[index]"
        class="input"
        type="number"
        maxlength="3"
        v-bind:class="{ 'is-info': isActive, 'is-danger': !isValid, 'is-primary': isValid }"
        v-bind:placeholder="placeholderPos(index)"
        @focus="ipFocus(index)"
        @blur="blur"
      >
    </div>
  </div>
</template>

<script>
export default {
  name: 'InputIP',
  model: {
    prop: 'ip',
    event: 'change'
  },
  props: {
    ip: {
      required: true,
      type: String
    },
    placeholder: {
      type: String,
      default: () => ''
    }
  },
  data () {
    return {
      ipCopy: [0, 0, 0, 0],
      isActive: false,
      isValid: false
    }
  },
  watch: {
    ip (newIp) {
      this.copyValue(newIp)
      this.changed()
    }
  },
  beforeMount () {
    this.copyValue(this.ip)
  },
  methods: {
    placeholderPos (segment) {
      // No placeholder
      if (!this.placeholder) { return '' }
      // Dummy IP placeholder
      switch (segment) {
        case 0:
          return '192'
        case 1:
          return '168'
        case 2:
          return '0'
        case 3:
          return '1'
      }
    },
    ipFocus (index) {
      this.isActive = true
    },
    blur () {
      this.isActive = false
      this.changed()
    },
    copyValue (ipString) {
      if (ipString) { this.ipToArray(ipString) }

      this.isValid = this.validateIP(ipString)
      this.changed()
    },
    ipToArray (ip) {
      let segments = []
      ip.split('.').map(segment => {
        if (isNaN(segment) || segment < 0 || segment > 255) { segment = 255 }
        segments.push(segment)
      })
      if (segments.length !== 4) {
        this.clearAll()
      } else { this.ipCopy = segments }
    },
    arrayToIp (ipArray) {
      return ipArray.join('.')
    },
    validateIP (ipString) {
      return (/^(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$/.test(ipString))
    },
    changed () {
      this.$emit('change', this.arrayToIp(this.ipCopy))
    },
    clearAll () {
      this.ipCopy = ['', '', '', '']
      this.isValid = false
    }
  }
}
</script>

<style lang="scss" scoped>
  .input-wrapper {
    display: inline-flex;
    justify-content: center;
    width: 100%;
    .segment {
      padding: 0 5px;
    }
    .input {
      width: 65px;
    }
  }
</style>
