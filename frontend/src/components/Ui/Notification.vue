<template>
  <transition name="fade">
    <div
      v-show="showMessage"
      class="toast-wrapper"
    >
      <div
        class="toast notification"
        v-bind:class="notifyStyle"
      >
        {{ message }}
      </div>
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
    -webkit-box-shadow: #dedede 2px 2px 2px;
    box-shadow: #dedede 2px 2px 2px;
    padding: 1em 3em;
  }
  .toast-wrapper {
    width: 300px;
    height: 300px;
    margin: 0 auto;
    position: fixed;
    left: 50%;
    top: 50%;
    margin-left: -150px;
    margin-top: -150px;
    z-index: 9999;
  }
</style>
