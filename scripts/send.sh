#!/bin/bash
tty=/dev/pts/2
size=`stat --printf="%s" $1`
echo $size | sudo tee $tty
sudo tee $tty < $1   
