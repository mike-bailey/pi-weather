1. raspi-config enable camera
Standard ras@si has only h264 fromat, no conversion to mp4.

Solution:
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
