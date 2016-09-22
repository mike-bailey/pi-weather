#!/bin/bash

date

# --------------------------------
# Copy the camera image into place
# --------------------------------

cp /opt/raspi/data/raspicam.jpg /srv/www/std-root/fm4dd.com/misc/images


# --------------------------------
# Check if sensor data is stale
# --------------------------------

cat /opt/raspi/data/sensor.txt
TIME=`cat /opt/raspi/data/sensor.txt | cut -d " " -f 1`
if [ "$TIME" == "" ]; then
  echo "Error getting timestamp from sensor.txt"
  exit
fi

OLDTIME=`/usr/bin/rrdtool last /opt/raspi/data/am2302.rrd`
if [ "$TIME" = "$OLDTIME" ]; then
  echo "Error getting new sensor data: last update from: `date -d @$TIME`"
  exit
else
  echo "Received new sensor data, TS: `date -d @$TIME`"
fi

# --------------------------------
# Did we recover from a NW outage?
# --------------------------------

let TDIFF=$TIME-$OLDTIME
if [ $TDIFF -gt 90 ]; then
  echo "Recovered from approx. $TDIFF seconds network outage."
fi

TEMP=`cat /opt/raspi/data/sensor.txt | cut -d " " -f 2 | cut -c 6- | cut -d "*" -f 1`
if [ "$TEMP" == "" ]; then
  echo "Error getting temperature from sensor.txt"
  exit
fi

HUMI=`cat /opt/raspi/data/sensor.txt | cut -d " " -f 3 | cut -c 10- | cut -d "%" -f 1`
if [ "$HUMI" == "" ]; then
  echo "Error getting humidity from sensor.txt"
  exit
fi

REAL=`/opt/raspi/data/thicalc $TEMP $HUMI`

# --------------------------------
# write new data into the RRD DB
# --------------------------------

echo "/usr/bin/rrdtool update /opt/raspi/data/am2302.rrd $TIME:$TEMP:$HUMI:$REAL"
/usr/bin/rrdtool updatev /opt/raspi/data/am2302.rrd "$TIME:$TEMP:$HUMI:$REAL"

# --------------------------------
# Create the daily graph images
# --------------------------------

TEMPPNG=/srv/www/std-root/fm4dd.com/misc/images/am2302_temp.png
HUMIPNG=/srv/www/std-root/fm4dd.com/misc/images/am2302_humi.png
REALPNG=/srv/www/std-root/fm4dd.com/misc/images/am2302_real.png

/usr/bin/rrdtool graph $TEMPPNG -a PNG \
  --start -16h --title=Temperature \
  --border=0  \
  --step=60s  \
  --width=419 \
  --height=77 \
  'DEF:temp1=/opt/raspi/data/am2302.rrd:temp:AVERAGE' \
  'AREA:temp1#FF0000:Temperature' \
  'GPRINT:temp1:MIN:Min\: %3.1lf°C' \
  'GPRINT:temp1:MAX:Max\: %3.1lf°C' \
  'GPRINT:temp1:LAST:Last\: %3.1lf°C'

/usr/bin/rrdtool graph $HUMIPNG -a PNG \
  --start -16h --title=Humidity \
  --border=0  \
  --step=60s  \
  --upper-limit=100 \
  --lower-limit=0 \
  --width=419 \
  --height=77 \
  'DEF:humi1=/opt/raspi/data/am2302.rrd:humi:AVERAGE' \
  'AREA:humi1#0000FF:Humidity' \
  'GPRINT:humi1:MIN:Min\: %3.1lf%%' \
  'GPRINT:humi1:MAX:Max\: %3.1lf%%' \
  'GPRINT:humi1:LAST:Last\: %3.1lf%%'

/usr/bin/rrdtool graph $REALPNG -a PNG \
  --start -16h --title='THI "Real Feel"' \
  --border=0  \
  --step=60s  \
  --width=419 \
  --height=77 \
  'DEF:real1=/opt/raspi/data/am2302.rrd:real:AVERAGE' \
  'AREA:real1#00C900:THI in °C' \
  'GPRINT:real1:MIN:Min\: %3.1lf°C' \
  'GPRINT:real1:MAX:Max\: %3.1lf°C' \
  'GPRINT:real1:LAST:Last\: %3.1lf°C'

# --------------------------------
# Create the monthly graph images
# --------------------------------
# --x-grid GTM:GST:MTM:MST:LTM:LST:LPR:LFM
# GTM:GST base grid (Unit:How Many)
# MTM:MST major grid (Unit:How Many)
# LTM:LST how often labels are placed

MTEMPPNG=/srv/www/std-root/fm4dd.com/misc/images/monthly_temp.png
MHUMIPNG=/srv/www/std-root/fm4dd.com/misc/images/monthly_humi.png
MREALPNG=/srv/www/std-root/fm4dd.com/misc/images/monthly_real.png

/usr/bin/rrdtool graph $MTEMPPNG -a PNG \
  --start end-21d --end 00:00 \
  --title=Temperature \
  --x-grid HOUR:8:DAY:1:DAY:1:86400:%d \
  --width=619 \
  --height=77 \
  --border=1  \
  --color SHADEA#000000 \
  --color SHADEB#000000 \
  'DEF:temp1=/opt/raspi/data/am2302.rrd:temp:AVERAGE' \
  'AREA:temp1#FF0000:Temperature' \
  'GPRINT:temp1:MIN:Min\: %3.1lf°C' \
  'GPRINT:temp1:MAX:Max\: %3.1lf°C' \
  'GPRINT:temp1:LAST:Last\: %3.1lf°C'


/usr/bin/rrdtool graph $MHUMIPNG -a PNG \
  --start end-21d --end 00:00 \
  --title=Humidity \
  --x-grid HOUR:8:DAY:1:DAY:1:86400:%d \
  --upper-limit=100 \
  --lower-limit=0 \
  --width=619 \
  --height=77 \
  --border=1  \
  --color SHADEA#000000 \
  --color SHADEB#000000 \
  'DEF:humi1=/opt/raspi/data/am2302.rrd:humi:AVERAGE' \
  'AREA:humi1#0000FF:Humidity' \
  'GPRINT:humi1:MIN:Min\: %3.1lf%%' \
  'GPRINT:humi1:MAX:Max\: %3.1lf%%' \
  'GPRINT:humi1:LAST:Last\: %3.1lf%%'


/usr/bin/rrdtool graph $MREALPNG -a PNG \
  --start end-21d --end 00:00 \
  --title='THI "Real Feel"' \
  --x-grid HOUR:8:DAY:1:DAY:1:86400:%d \
  --width=619 \
  --height=77 \
  --border=1  \
  --color SHADEA#000000 \
  --color SHADEB#000000 \
  'DEF:real1=/opt/raspi/data/am2302.rrd:real:AVERAGE' \
  'AREA:real1#00C900:THI in °C' \
  'GPRINT:real1:MIN:Min\: %3.1lf°C' \
  'GPRINT:real1:MAX:Max\: %3.1lf°C' \
  'GPRINT:real1:LAST:Last\: %3.1lf°C'

# --------------------------------
# Create the yearly graph images
# --------------------------------

YTEMPPNG=/srv/www/std-root/fm4dd.com/misc/images/yearly_temp.png
YHUMIPNG=/srv/www/std-root/fm4dd.com/misc/images/yearly_humi.png
YREALPNG=/srv/www/std-root/fm4dd.com/misc/images/yearly_real.png

/usr/bin/rrdtool graph $YTEMPPNG -a PNG \
  --start end-2y --end 00:00 \
  --x-grid MONTH:1:YEAR:1:MONTH:1:0:%m \
  --title=Temperature \
  --width=619 \
  --height=77 \
  --border=1  \
  --color SHADEA#000000 \
  --color SHADEB#000000 \
  'DEF:temp1=/opt/raspi/data/am2302.rrd:temp:AVERAGE' \
  'AREA:temp1#FF0000:Temperature' \
  'GPRINT:temp1:MIN:Min\: %3.1lf°C' \
  'GPRINT:temp1:MAX:Max\: %3.1lf°C' \
  'GPRINT:temp1:LAST:Last\: %3.1lf°C'


/usr/bin/rrdtool graph $YHUMIPNG -a PNG \
  --start end-2y --end 00:00 \
  --x-grid MONTH:1:YEAR:1:MONTH:1:0:%m \
  --title=Humidity \
  --upper-limit=100 \
  --lower-limit=0 \
  --width=619 \
  --height=77 \
  --border=1  \
  --color SHADEA#000000 \
  --color SHADEB#000000 \
  'DEF:humi1=/opt/raspi/data/am2302.rrd:humi:AVERAGE' \
  'AREA:humi1#0000FF:Humidity' \
  'GPRINT:humi1:MIN:Min\: %3.1lf%%' \
  'GPRINT:humi1:MAX:Max\: %3.1lf%%' \
  'GPRINT:humi1:LAST:Last\: %3.1lf%%'


/usr/bin/rrdtool graph $YREALPNG -a PNG \
  --start end-2y --end 00:00 \
  --x-grid MONTH:1:YEAR:1:MONTH:1:0:%m \
  --title='THI "Real Feel"' \
  --width=619 \
  --height=77 \
  --border=1  \
  --color SHADEA#000000 \
  --color SHADEB#000000 \
  'DEF:real1=/opt/raspi/data/am2302.rrd:real:AVERAGE' \
  'AREA:real1#00C900:THI in °C' \
  'GPRINT:real1:MIN:Min\: %3.1lf°C' \
  'GPRINT:real1:MAX:Max\: %3.1lf°C' \
  'GPRINT:real1:LAST:Last\: %3.1lf°C'

