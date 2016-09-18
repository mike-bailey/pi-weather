#!/bin/sh -e
#
# blink.sh:
#	Standard "blink" program in wiringPi. Blinks an LED connected
#	to the first GPIO pin.
#
# Copyright (c) 2012-2013 Gordon Henderson. <projects@drogon.net>
#######################################################################
# This file is part of wiringPi:
#	https://projects.drogon.net/raspberry-pi/wiringpi/
#
#    wiringPi is free software: you can redistribute it and/or modify
#    it under the terms of the GNU Lesser General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    wiringPi is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU Lesser General Public License for more details.
#
#    You should have received a copy of the GNU Lesser General Public License
#    along with wiringPi.  If not, see <http://www.gnu.org/licenses/>.
#######################################################################

# LED Pin - wiringPi pin 0 is BCM_GPIO 17.

PIN0=0
PIN1=2
PIN2=3

gpio mode $PIN0 in
gpio mode $PIN1 out
gpio mode $PIN2 out

while true; do
  button=`gpio read $PIN0`
  # if [ $button < 1 ]; then
 
  gpio write $PIN1 1
  gpio write $PIN2 0
  sleep 0.5
  gpio write $PIN1 0
  gpio write $PIN2 1
  sleep 0.5 
# fi
done
