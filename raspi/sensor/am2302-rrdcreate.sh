#!/bin/bash

# The RRD database will hold three data sources:
# ----------------------------------------------
# 1. Air temperature in degree Celsius
# 2. Relative humidity in percent
# 3. temperature-humidity index (THI) in degree Celsius

# Data slots are allocated as follows:
# ------------------------------------
# RRA:AVERAGE:0.5:1:14400 # store one minute reading in 14400 entries (10 days)
# RRA:AVERAGE:0.5:60:17568 # store one 60min avg in 17568 entries (732 days = 2 years)
# RRA:AVERAGE:0.5:1440:7320 # store one day avg in 7320 entries (20 years)

rrdtool create /home/pi/sensor/rrd/am2302.rrd    \
--start now --step 60s       \
DS:temp:GAUGE:300:-100:100   \
DS:humi:GAUGE:300:0:100      \
DS:real:GAUGE:300:-100:100   \
RRA:AVERAGE:0.5:1:14400      \
RRA:AVERAGE:0.5:60:17568     \
RRA:AVERAGE:0.5:1440:7320    \
RRA:MIN:0.5:60:17568         \
RRA:MAX:0.5:60:17568         \
RRA:MIN:0.5:1440:7320        \
RRA:MAX:0.5:1440:7320
