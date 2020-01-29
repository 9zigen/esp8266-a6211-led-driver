<template>
  <input
    class="input-text"
    type="number"
    v-bind:value="value"
    v-bind:minlength="minlength"
    v-bind:maxlength="maxlength"
    v-bind:placeholder="placeholder"
    @input="input($event.target.value)"
    @blur="input($event.target.value)"
  >
</template>

<script>
export default {
  name: 'InputNumber',
  props: {
    value: {
      type: [String, Number],
      default: () => '1'
    },
    placeholder: {
      type: String,
      default: () => ''
    },
    minlength: {
      type: String,
      default: () => '0'
    },
    maxlength: {
      type: String,
      default: () => '1'
    }
  },
  data () {
    return {
      isValid: false
    }
  },
  methods: {
    input (value) {
      this.$emit('input', this.validRange(value))
    },
    validRange (value) {
      var pattern = '\\d{' + this.minlength + ',' + this.maxlength + '}'
      var regex = new RegExp(pattern)
      let match = value.match(regex)
      return match[0]
    }
  }
}
</script>

<style lang="scss" scoped>

</style>
