<template>
  <div class="range-slider">
    <input class="range-slider__range" step="1" v-bind:min="min" v-bind:max="max" type="range" v-bind:value="value" v-on:change="$emit('change', $event.target.value)">
    <span class="range-slider__value">{{value}}</span>
  </div>
</template>

<script>
export default {
  name: 'InputSlider',
  model: {
    prop: 'value',
    event: 'change'
  },
  props: {
    value: Number,
    min: String,
    max: String
  }
}
</script>

<style lang="scss" scoped>
  // Base Colors
  $shade-10: #2c3e50 !default;
  $shade-1: #209cee !default;
  $shade-0: #fff !default;
  $teal: #1abc9c !default;

  .range-slider {
    margin: 0;
  }

  // Range Slider
  $range-width: 100% !default;

  $range-handle-color: $shade-10 !default;
  $range-handle-color-hover: $teal !default;
  $range-handle-size: 20px !default;

  $range-track-color: $shade-1 !default;
  $range-track-height: 10px !default;

  $range-label-color: $shade-10 !default;
  $range-label-width: 60px !default;

  .range-slider {
    width: $range-width;
  }

  .range-slider__range {
    -webkit-appearance: none;
    width: calc(100% - (#{$range-label-width + 13px}));
    height: $range-track-height;
    border-radius: 5px;
    background: $range-track-color;
    outline: none;
    padding: 0;
    margin: 0;

    // Range Handle
    &::-webkit-slider-thumb {
      appearance: none;
      width: $range-handle-size;
      height: $range-handle-size;
      border-radius: 50%;
      background: $range-handle-color;
      cursor: pointer;
      transition: background .15s ease-in-out;

      &:hover {
        background: $range-handle-color-hover;
      }
    }

    &:active::-webkit-slider-thumb {
      background: $range-handle-color-hover;
    }

    &::-moz-range-thumb {
      width: $range-handle-size;
      height: $range-handle-size;
      border: 0;
      border-radius: 50%;
      background: $range-handle-color;
      cursor: pointer;
      transition: background .15s ease-in-out;

      &:hover {
        background: $range-handle-color-hover;
      }
    }

    &:active::-moz-range-thumb {
      background: $range-handle-color-hover;
    }

    // Focus state
    &:focus {

      &::-webkit-slider-thumb {
        box-shadow: 0 0 0 3px $shade-0,
        0 0 0 6px $teal;
      }
    }
  }

  // Range Label
  .range-slider__value {
    display: inline-block;
    position: relative;
    width: $range-label-width;
    color: $shade-0;
    line-height: 20px;
    text-align: center;
    border-radius: 3px;
    background: $range-label-color;
    padding: 5px 10px;
    margin-left: 8px;

    &:after {
      position: absolute;
      top: 8px;
      left: -7px;
      width: 0;
      height: 0;
      border-top: 7px solid transparent;
      border-right: 7px solid $range-label-color;
      border-bottom: 7px solid transparent;
      content: '';
    }
  }


  // Firefox Overrides
  ::-moz-range-track {
    background: $range-track-color;
    border: 0;
  }

  input::-moz-focus-inner,
  input::-moz-focus-outer {
    border: 0;
  }

</style>
