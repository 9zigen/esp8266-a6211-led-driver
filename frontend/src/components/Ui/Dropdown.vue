<template>
  <div class="navbar-item has-dropdown" v-bind:class="showMenu" v-on:click="active = !active">
    <a class="navbar-link">
      More
    </a>
    <div class="navbar-dropdown" v-bind:class="boxed? 'is-boxed':''">
      <slot></slot>
    </div>
  </div>
</template>

<script>
export default {
  name: 'Dropdown',
  props: {
    boxed: Boolean
  },
  data: function () {
    return {
      active: false
    }
  },
  computed: {
    showMenu: function () {
      return this.active ? 'is-active' : ''
    }
  }
}
</script>

<style scoped>

</style>
