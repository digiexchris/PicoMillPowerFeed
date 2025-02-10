#!/bin/bash
cd nuttx
#./tools/configure.sh esp32s3-devkit:usbnsh
./tools/configure.sh raspberrypi-pico:usbnsh
#cmake -B build -DBOARD_CONFIG=raspberrypi-pico:usbnsh
cd ..