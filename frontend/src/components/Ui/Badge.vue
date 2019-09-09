<template>
  <span class="badge">{{ text }}</span>
</template>

<script>
export default {
  name: 'Badge',
  props: {
    text: {
      type: [String, Number],
      required: true
    }
  }
}
</script>

<style lang="scss" scoped>
  .badge {
    background: #f3f3f3;
    color: #2c3e50;
    padding: 2px 5px;
    font-size: 12px;
    line-height: 18px;
    margin: 0 4px;
    border-radius: 10px;
    .info {

    }
  }
</style>
