# am-to-fm-radio-conversion

[![GitHub version](https://img.shields.io/github/v/release/ldab/am-to-fm-radio-conversion?include_prereleases)](https://github.com/ldab/am-to-fm-radio-conversion/releases/latest)
![Build Status](https://github.com/ldab/am-to-fm-radio-conversion/actions/workflows/workflow.yml/badge.svg)
[![License: MIT](https://img.shields.io/badge/License-MIT-green.svg)](https://github.com/ldab/am-to-fm-radio-conversion/blob/master/LICENSE)

[![GitHub last commit](https://img.shields.io/github/last-commit/ldab/am-to-fm-radio-conversion.svg?style=social)](https://github.com/ldab/am-to-fm-radio-conversion)

<p align="center">
  <img src="./github/sample.jpeg" width="30%">
</p>

### TODO

- [ ] PCB
- [ ] Potentially Transformerless

### Main CPU

I used the the following board because that is what I had on my drawer:

<a href="https://www.adafruit.com/product/3500">
<p align="center">
  <img src="https://cdn-learn.adafruit.com/assets/assets/000/110/617/original/adafruit_products_Adafruit_Trinket_M0_pinout.png?1649362369" width="50%">
</p>
</a>

### FM Radio receiver

<a href="https://www.sparkfun.com/products/11083">
<p align="center">
  <img src="https://cdn.sparkfun.com//assets/parts/6/2/3/5/11083-04.jpg" width="50%">
</p>
</a>

### Using the Variable Capacitor to select the frequency

If we connect the variable to the internal stray capacitor, the voltage on the ADC pin will settle quickly (few pico-seconds), therefore it is a simple voltage divider with stray capacitance. If you don't believe me, you can simulate it here: https://tinyurl.com/2p23hk4f

<p align="center">
  <img src=".github\sim.png" width="50%">
</p>

Or calculate here: https://tinyurl.com/yc6bk4t7

<p align="center">
  <img src=".github\plot.png" width="50%">
</p>

### FM Radio receiver

<p align="center">
  <img src=".github\power.png" width="50%">
</p>