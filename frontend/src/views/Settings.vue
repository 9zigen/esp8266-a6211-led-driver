<template>
  <div class="container">
    <div class="columns is-marginless">
      <div class="column">
        <div class="content has-text-left">
          <h1 class="title">
            Services Settings
          </h1>
        </div>
      </div>
    </div>

    <div class="columns is-marginless">
      <div class="column">
        <!-- MAIN -->
        <div class="notification bg-notification is-light">
          <div class="buttons has-addons">
            <span
              v-for="tab in tabs"
              v-bind:key="tab"
              v-bind:class="['button', { 'is-primary': currentTab === tab }]"
              @click="currentTab = tab"
            >{{ tab }}</span>
          </div>
        </div>
      </div>
    </div>

    <!-- Dynamic component -->
    <component v-bind:is="currentTabComponent" />
  </div>
</template>

<script>
import TabServices from './components/Services'
import TabLeds from './components/Leds'

export default {
  name: 'Settings',
  components: {
    TabServices,
    TabLeds
  },
  data () {
    return {
      currentTab: 'Services',
      tabs: ['Services', 'Leds']
    }
  },
  computed: {
    currentTabComponent () {
      return `tab-${this.currentTab.toLowerCase()}`
    }
  }
}
</script>
