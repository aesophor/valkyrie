#!/usr/bin/env bash

tty=/dev/ttyUSB0
size=$(stat --printf="%s" "$1")
echo "$size" | sudo tee "$tty"
sudo tee "$tty" < "$1"
