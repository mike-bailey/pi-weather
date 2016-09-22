#!/bin/bash
# -------------------------------------------------------- #
# Create IP info file to be send to the server, helping to #
# find the Pi on the DHCP network, the Wifi RTR doesn't do #
# static assignments. Only write the file if it isn't      #
# there (PI reboot), or if the IP changed (WiFi reboot).   #
# -------------------------------------------------------- #
IPINFO=`ifconfig wlan0 | grep 'inet addr'`

if [ ! -f /tmp/raspinet.txt ]; then
  echo $IPINFO > /tmp/raspinet.txt
fi

OLDINFO=`cat /tmp/raspinet.txt`

if [ ! "$OLDINFO" = "$IPINFO" ]; then
  echo $IPINFO > /tmp/raspinet.txt
fi

# -------------------------------------------------------- #
# Take the camera picture                                  #
# -------------------------------------------------------- #
raspistill -w 640 -h 480 -q 80 -o /tmp/raspicam.jpg

# -------------------------------------------------------- #
# Take the temperature and humidity sensor reading, and    #
# add it to the output file send to the server. Create a   #
# backup file with the last 60 readings, used in case of   #
# any network outage (covering up to one hour). Normally   #
# the server picks only te sensor.txt. If there was a gap  #
# after outages, the backup file get processed as well.    #
# -------------------------------------------------------- #
SENSORDATA=`python /home/pi/sensor/Adafruit_Python_DHT/examples/simpletest.py`
echo  $SENSORDATA > /tmp/sensor.txt;

echo  $SENSORDATA >> /tmp/backup.txt;
LENGTH=`wc -l /tmp/backup.txt | cut -d " " -f 1,1`
if [ $LENGTH -gt 60 ]; then
  # delete the first (oldest) line from file
  sed --in-place '1d' /tmp/backup.txt
fi

# -------------------------------------------------------- #
# Copy the four files /tmp/raspinet.txt, /tmp/raspicam.jpg #
# /tmp/sensor.txt and /tmp/backup.txt to the server.       #
# -------------------------------------------------------- #
sftp -b /home/pi/sftp.bat raspi@fm4dd.com > /dev/null
