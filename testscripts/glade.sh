#!/bin/bash
sudo ln -sf $(readlink -f .bin/Debug/libv3270.so.5.2) /usr/lib64/libv3270.so.5.2

glade glade/widget-test.glade


