#!/bin/sh

# Setup serial console access
vi -c "/ttyS0/d" -c ":wq" /etc/inittab 1>/dev/null 2>&1
echo "T0:23:respawn:/sbin/agetty -L ttyS0 9600 vt100" >> /etc/inittab

vi -c "/ttyS1/d" -c ":wq" /etc/inittab 1>/dev/null 2>&1
echo "T1:23:respawn:/sbin/agetty -L ttyS1 9600 vt100"$'\n' >> /etc/inittab
