<template>
  <label class="input-label">
    <input
      class="input-text"
      type="text"
    >
  </label>
</template>

<script>
export default {
  name: 'InputText',
  props: {
    placeholder: {
      type: String,
      default: () => ''
    }
  }
}
</script>

<style lang="scss" scoped>

</style>
