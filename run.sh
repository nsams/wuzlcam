nc -l -p 5001 | ./build/wuzlcam &
#nc -l -p 5001 | mplayer -cache 512 -fps 30 - &
ssh pi@192.168.0.226 "raspivid -t 999999 -o - -w 640 -h 360 -fps 90 | nc 192.168.0.13 5001"
#ssh pi@192.168.0.226 "raspivid -t 999999 -o - -w 1296 -h 730 -fps 49 | nc 192.168.0.13 5001"
killall wuzlcam 2>/dev/null
