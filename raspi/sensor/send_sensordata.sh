#!/bin/bash
ifconfig wlan0 | grep 'inet addr' > /tmp/raspinet.txt
raspistill -w 640 -h 480 -q 80 -o /tmp/raspicam.jpg
python /home/pi/sensor/Adafruit_Python_DHT/examples/simpletest.py > /tmp/sensor.txt;
sftp -b /home/pi/sftp.bat raspi@fm4dd.com > /dev/null
