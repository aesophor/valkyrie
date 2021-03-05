#!/usr/bin/env python3
# -*- encoding: utf-8 -*-
import sys

def usage():
    return 'usage: {} <kernel_img_path>'.format(sys.argv[0])

def main():
    if len(sys.argv) < 2:
        print(usage())
        sys.exit(0)

    with open(sys.argv[1]) as f:
        kernel = f.read()

    with open('/dev/ttyUSB0', 'wb', buffering = 0) as tty:
        tty.write(kernel)

if __name__ == '__main__':
    main()
