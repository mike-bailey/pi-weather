Raspi Camera

1. raspi-config enable camera

Standard raspi can do only h264, no conversion to mp4.  Solution:

2. download ffmpeg_3.1.1-1_armhf.deb
wget https://github.com/ccrisan/motioneye/wiki/precompiled/ffmpeg_3.1.1-1_armhf.deb

3. Install ffmpeg library
dpkg -i ffmpeg_3.1.1-1_armhf.deb

4. install dependend libs
 apt-get install curl libx264-142

5. test video
raspivid -t 10000 -o test.h264

6. convert h264 to mp4
ffmpeg -r 30 -i test.h264 -vcodec copy test.mp4

Sensor driver software:

1. git clone https://github.com/adafruit/Adafruit_Python_DHT.git
2. cd Adafruit_Python_DHT
3. sudo apt-get install build-essential python-dev
4. sudo python setup.py install
5. cd examples
6. vi simple4. cd examples
7. vi simpletest.py and set the sensor data pin (e.g. 22)
8. root@raspi2:/home/pi/sensor/Adafruit_Python_DHT/examples# python simpletest.py
Temp=26.7*C  Humidity=47.4%

manual test:
python sensor/Adafruit_Python_DHT/examples/simpletest.py | ssh fm@fm4dd.com "cat > /opt/raspi/data/sensor.txt"

pi@raspi2:~ $ cat /etc/crontab |grep sensor
*  *    * * *   pi      /srv/scripts/send_sensordata.sh
