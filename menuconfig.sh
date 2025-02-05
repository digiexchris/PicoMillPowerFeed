#!/bin/bash
cd nuttx
# ln -s ../.config ./
export PICO_SDK_PATH=/tools/pico/pico-sdk
make menuconfig
make export
cd ..
tar -xzf nuttx/nuttx-export-12.8.0.tar.gz
