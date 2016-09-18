1. git clone https://github.com/adafruit/Adafruit_Python_DHT.git
2. cd Adafruit_Python_DHT
3. sudo python setup.py install
4. cd examples
5. vi simple4. cd examples
6. vi simpletest.py and set the sensor data pin (e.g. 22)
7. root@raspi2:/home/pi/sensor/Adafruit_Python_DHT/examples# python simpletest.py
Temp=26.7*C  Humidity=47.4%

python sensor/Adafruit_Python_DHT/examples/simpletest.py | ssh fm@fm4dd.com "cat > /tmp/raspi.dat"
