#!/bin/bash
cd nuttx
# ln -s ../.config ./
make menuconfig
make export
cd ..
tar -xzf nuttx/nuttx-export-12.8.0.tar.gz
