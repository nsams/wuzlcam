nc -l -p 5001 | ./build/opencvtest &
ssh pi@192.168.0.27 "raspivid -t 999999 -o - -w 640 -h 480 -fps 90 | nc 192.168.0.13 5001"
#ssh pi@192.168.0.27 "raspivid -t 999999 -o - -w 1296 -h 730 -fps 49 | nc 192.168.0.13 5001"
