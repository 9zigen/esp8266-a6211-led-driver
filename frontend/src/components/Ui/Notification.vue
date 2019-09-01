<template>
  <transition name="fade">
    <div class="toast notification" v-bind:class="notifyStyle" v-show="showMessage">
      {{message}}
    </div>
  </transition>

</template>

<script>
export default {
  name: 'Notification',
  props: {
    type: {
      validator: function (value) {
        return ['dark', 'primary', 'link', 'info', 'success', 'warning', 'danger'].indexOf(value) !== -1
      }
    },
    message: String,
    showMessage: Boolean
  },
  computed: {
    notifyStyle () {
      return 'is-' + this.type
    }
  }
}
</script>

<style scoped>
  .fade-enter-active, .fade-leave-active {
    channel_transition: opacity .5s;
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
  }
</style>
