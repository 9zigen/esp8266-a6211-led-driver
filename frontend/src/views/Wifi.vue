<template>
  <div class="container">
    <h1 class="title is-5">WiFi Settings</h1>
    <h2 class="subtitle is-7">SSID, IP Address, DHCP</h2>

    <div class="columns is-centered">
      <div class="column is-10-mobile is-offset-1-mobile is-8">

        <!-- Start Networks -->
        <div class="columns is-vcentered" v-for="network in networks" :key="network.id">
          <div class="column is-12">
            <div class="field is-horizontal">
              <div class="field-label is-normal">
                <label class="label">SSID #{{network.id + 1}}</label>
              </div>
              <div class="field-body">
                <div class="field">
                  <div class="control">
                    <input class="input" type="text" placeholder="WiFi Network Name" v-model="network.ssid">
                  </div>
                  <p class="help">Network Name</p>
                </div>
                <div class="field">
                  <div class="control">
                    <input class="input" type="text" placeholder="Password" v-model="network.password">
                  </div>
                  <p class="help">Password</p>
                </div>
                <div class="field">
                  <div class="control has-text-centered">
                    <toggle-switch v-model="network.dhcp"></toggle-switch>
                  </div>
                  <p class="help">DHCP</p>
                </div>
                <div class="field">
                  <div class="control has-text-centered">
                    <button v-on:click="deleteNetwork(network.id)" class=" button is-danger"><span>Delete</span></button>
                  </div>
                </div>
              </div>
            </div>
            <transition name="list">
              <div class="columns is-centered" v-show="!network.dhcp">
                <div class="column is-12">
                  <div class="field is-horizontal">
                    <div class="field-label is-normal">
                      <label class="label">Network:</label>
                    </div>
                    <div class="field-body">
                      <div class="field">
                        <div class="control">
                          <input class="input" type="text" placeholder="192.168.001.002" v-model="network.ip_address" v-mask="'###.###.###.###'">
                        </div>
                        <p class="help">IP address</p>
                      </div>
                      <div class="field">
                        <div class="control">
                          <input class="input" type="text" placeholder="192.168.001.001" v-model="network.gateway" v-mask="'###.###.###.###'">
                        </div>
                        <p class="help">Gateway</p>
                      </div>
                      <div class="field">
                        <div class="control">
                          <input class="input" type="text" placeholder="255.255.255.000" v-model="network.mask" v-mask="'###.###.###.###'">
                        </div>
                        <p class="help">Mask</p>
                      </div>
                      <div class="field">
                        <div class="control">
                          <input class="input" type="text" placeholder="192.168.001.001" v-model="network.dns" v-mask="'###.###.###.###'">
                        </div>
                        <p class="help">DNS</p>
                      </div>
                    </div>
                  </div>
                </div>
              </div>
            </transition>
          </div>
        </div>
        <!-- End Networks -->

      </div>
    </div>

    <div class="columns is-vcentered">
      <div class="column is-12">
        <div class="field is-grouped is-grouped-centered">
          <div class="control">
            <a class="button is-primary" v-on:click="saveSettings">Apply</a>
          </div>
          <div class="control">
            <a class="button is-info" v-on:click="addNetwork">Add New</a>
          </div>
        </div>
      </div>
    </div>
  </div>
</template>

<script>

import { store, mutations } from '../store'
import { eventBus } from '@/eventBus'
import { api } from '@/api'

export default {
  name: 'wifi',
  data: function () {
    return {
      networks: [],
      nextNetworkId: 0,
      capacity: 0
    }
  },
  methods: {
    loadSettings () {
      if (store.settings.networks) {
        this.networks = store.settings.networks.items.map((value, index) => {
          return {
            id: index,
            ssid: value.ssid,
            password: value.password,
            ip_address: value.ip_address,
            mask: value.mask,
            gateway: value.gateway,
            dns: value.dns,
            dhcp: value.dhcp
          }
        })
        this.nextNetworkId = this.networks.length

        /* set max available network config, this is Hardware specific */
        if (store.settings.networks.capacity > 0)
          this.capacity = store.settings.networks.capacity
      }

    },
    saveSettings () {
      mutations.setSettings({ networks: { items: this.networks } })
      api.setNetworks()
    },
    addNetwork () {
      if (this.networks.length < this.capacity) {
        this.networks.push({
          id: this.nextNetworkId,
          ssid: '',
          password: '',
          ip_address: '192.168.001.100',
          mask: '255.255.255.0',
          gateway: '192.168.001.001',
          dns: '192.168.001.001',
          dhcp: true
        })
        this.nextNetworkId++
      } else {
        /* error - MAX Networks reached */
        eventBus.$emit('message', 'MAX Networks reached', 'danger')
      }

    },
    deleteNetwork (id) {
      this.networks.splice(id, 1)
      if (this.nextNetworkId > 0) {
        this.nextNetworkId--
      }
    }
  },
  mounted () {
    eventBus.$on('networksLoaded', id => {
      this.loadSettings()
    })
  }
}
</script>

<style scoped lang="scss">
  .list-item {
    display: inline-block;
    margin-right: 10px;
  }
  .list-enter-active {
    channel_transition: all .5s ease-in;
  }
  .list-leave-active {
    channel_transition: all .5s ease-out;
  }
  .list-enter, .list-leave-to /* .list-leave-active до версии 2.1.8 */ {
    opacity: 0;
    transform: translateX(-30px);
  }
</style>
