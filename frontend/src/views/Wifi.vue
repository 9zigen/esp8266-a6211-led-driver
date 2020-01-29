<template>
  <div class="container">
    <div class="columns is-marginless">
      <div class="column">
        <div class="content has-text-left">
          <h1 class="title">
            WiFi Settings
          </h1>
        </div>
      </div>
    </div>

    <!-- Wifi -->
    <div class="columns is-marginless">
      <div class="column">
        <transition-group
          name="list"
          appear
        >
          <div
            v-for="network in networks"
            v-bind:key="network.id"
            class="columns"
          >
            <div class="column">
              <div class="notification bg-notification is-light">
                <div class="field is-horizontal">
                  <div class="field-label is-normal">
                    <label class="label">SSID #{{ network.id + 1 }}</label>
                  </div>
                  <div class="field-body">
                    <div class="field">
                      <div class="control">
                        <input
                          v-model="network.ssid"
                          class="input"
                          type="text"
                          placeholder="WiFi Network Name"
                        >
                      </div>
                      <p class="help">
                        Network Name
                      </p>
                    </div>
                    <div class="field">
                      <div class="control">
                        <input
                          v-model="network.password"
                          class="input"
                          type="text"
                          placeholder="Password"
                        >
                      </div>
                      <p class="help">
                        Password
                      </p>
                    </div>
                    <div class="field">
                      <div class="control has-text-centered">
                        <toggle-switch
                          v-model="network.dhcp"
                          round
                        />
                      </div>
                      <p class="help">
                        DHCP
                      </p>
                    </div>
                    <div class="field">
                      <div class="control has-text-centered">
                        <span
                          class="button is-danger is-flex-mobile"
                          @click="deleteNetwork(network.id)"
                        >
                          <x-icon size="1.5x" /> Delete
                        </span>
                      </div>
                    </div>
                  </div>
                </div>

                <transition name="list">
                  <div
                    v-show="!network.dhcp"
                    class="columns is-centered"
                  >
                    <div class="column is-12">
                      <div class="field is-horizontal">
                        <div class="field-label is-normal">
                          <label class="label">Network:</label>
                        </div>
                        <div class="field-body">
                          <div class="field">
                            <div class="control">
                              <input-ip
                                v-model="network.ip_address"
                                placeholder="192.168.1.200"
                              />
                            </div>
                            <p class="help">
                              IP address
                            </p>
                          </div>
                          <div class="field">
                            <div class="control">
                              <input-ip
                                v-model="network.gateway"
                                placeholder="192.168.1.1"
                              />
                            </div>
                            <p class="help">
                              Gateway
                            </p>
                          </div>
                          <div class="field">
                            <div class="control">
                              <input-ip
                                v-model="network.mask"
                                placeholder="255.255.255.0"
                              />
                            </div>
                            <p class="help">
                              Mask
                            </p>
                          </div>
                          <div class="field">
                            <div class="control">
                              <input-ip
                                v-model="network.dns"
                                placeholder="192.168.1.1"
                              />
                            </div>
                            <p class="help">
                              DNS
                            </p>
                          </div>
                        </div>
                      </div>
                    </div>
                  </div>
                </transition>
              </div>
            </div>
          </div>
        </transition-group>
        <!-- End Networks -->
      </div>
    </div>

    <!-- Buttons -->
    <div class="buttons is-centered">
      <a
        class="button is-info"
        @click="addNetwork"
      ><plus-icon size="1.5x" />Add New</a>
      <a
        class="button is-primary"
        @click="saveNetwork"
      ><check-icon size="1.5x" /> Apply</a>
    </div>
  </div>
</template>

<script>

import { http } from '@/http'
import { eventBus } from '@/eventBus'

export default {
  name: 'Wifi',
  data () {
    return {
      networks: [],
      nextNetworkId: 0,
      capacity: 2
    }
  },
  mounted () {
    this.loadNetwork()
  },
  methods: {
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
    },
    async saveNetwork () {
      let networks = await http.post('/config/networks', { networks: this.networks })
      if (networks.data.save) {
        eventBus.$emit('message', 'Saved', 'success')
      } else {
        eventBus.$emit('message', 'NOT Saved', 'danger')
      }
    },
    async loadNetwork () {
      let response = await http.get('/config/networks')
      this.networks = response.data.networks
      this.nextNetworkId = this.networks.length
    }
  }
}
</script>
