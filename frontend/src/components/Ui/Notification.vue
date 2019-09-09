<template>
  <transition name="fade">
    <div
      v-show="showMessage"
      class="toast notification"
      v-bind:class="notifyStyle"
    >
      {{ message }}
    </div>
  </transition>
</template>

<script>
export default {
  name: 'Notification',
  props: {
    type: {
      validator (value) {
        return ['dark', 'primary', 'link', 'info', 'success', 'warning', 'danger'].indexOf(value) !== -1
      },
      default: () => 'primary'
    },
    message: {
      type: String,
      default: () => ''
    },
    showMessage: Boolean
  },
  computed: {
    notifyStyle () {
      return `is-${this.type}`
    }
  }
}
</script>

<style scoped>
  .fade-enter-active, .fade-leave-active {
    transform: translateY(0px);
    -webkit-transform: translateY(0px);
    transition: all ease-in .2s;
  }
  .fade-enter, .fade-leave-to {
    opacity: 0;
    transform: translateY(-20px);
    -webkit-transform: translateY(40px);
    transition: all ease-out .5s;
  }
  .toast {
    position: absolute;
    bottom: 10px;
    right: 10px;
    -webkit-box-shadow: #444444 4px 4px 4px;
    box-shadow: #444444 0 1px 20px;
    padding: 1em 3em;
    z-index: 9999;
  }
</style>
