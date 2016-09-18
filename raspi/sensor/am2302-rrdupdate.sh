#!/bin/bash

date
cat /tmp/sensor.txt

TIME=`cat /tmp/sensor.txt | cut -d " " -f 1`
TEMP=`cat /tmp/sensor.txt | cut -d " " -f 2 | cut -c 6- | cut -d "*" -f 1`
HUMI=`cat /tmp/sensor.txt | cut -d " " -f 4 | cut -c 10- | cut -d "%" -f 1`
REAL=`/home/pi/sensor/bin/thicalc $TEMP $HUMI`

echo "/usr/bin/rrdtool update /home/pi/sensor/rrd/am2302.rrd $TIME:$TEMP:$HUMI:$REAL"
/usr/bin/rrdtool updatev /home/pi/sensor/rrd/am2302.rrd "$TIME:$TEMP:$HUMI:$REAL"

TEMPPNG=/home/pi/sensor/images/am2302_temp.png

/usr/bin/rrdtool graph $TEMPPNG -a PNG \
  --start -16h --title=Temperature \
  --border=0  \
  --step=60s  \
  --width=419 \
  --height=77 \
  'DEF:temp1=/home/pi/sensor/rrd/am2302.rrd:temp:AVERAGE' \
  'AREA:temp1#FF0000:Temperature' \
  'GPRINT:temp1:MIN:Min\: %3.1lf°C' \
  'GPRINT:temp1:MAX:Max\: %3.1lf°C' \
  'GPRINT:temp1:LAST:Last\: %3.1lf°C'

HUMIPNG=/home/pi/sensor/images/am2302_humi.png

/usr/bin/rrdtool graph $HUMIPNG -a PNG \
  --start -16h --title=Humidity \
  --border=0  \
  --step=60s  \
  --upper-limit=100 \
  --lower-limit=0 \
  --width=419 \
  --height=77 \
  'DEF:humi1=/home/pi/sensor/rrd/am2302.rrd:humi:AVERAGE' \
  'AREA:humi1#0000FF:Humidity' \
  'GPRINT:humi1:MIN:Min\: %3.1lf%%' \
  'GPRINT:humi1:MAX:Max\: %3.1lf%%' \
  'GPRINT:humi1:LAST:Last\: %3.1lf%%'

REALPNG=/home/pi/sensor/images/am2302_real.png

/usr/bin/rrdtool graph $REALPNG -a PNG \
  --start -16h --title='THI "Real Feel"' \
  --border=0  \
  --step=60s  \
  --width=419 \
  --height=77 \
  'DEF:real1=/home/pi/sensor/rrd/am2302.rrd:real:AVERAGE' \
  'AREA:real1#00C900:THI in °C' \
  'GPRINT:real1:MIN:Min\: %3.1lf°C' \
  'GPRINT:real1:MAX:Max\: %3.1lf°C' \
  'GPRINT:real1:LAST:Last\: %3.1lf°C'
