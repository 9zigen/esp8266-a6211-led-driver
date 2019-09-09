<template>
  <div
    class="navbar-item has-dropdown"
    v-bind:class="showMenu"
    @click="active = !active"
  >
    <a class="navbar-link">
      More
    </a>
    <div
      class="navbar-dropdown"
      v-bind:class="boxed? 'is-boxed':''"
    >
      <slot />
    </div>
  </div>
</template>

<script>
export default {
  name: 'Dropdown',
  props: {
    boxed: Boolean
  },
  data () {
    return {
      active: false
    }
  },
  computed: {
    showMenu () {
      return this.active ? 'is-active' : ''
    }
  }
}
</script>

<style scoped>

</style>
