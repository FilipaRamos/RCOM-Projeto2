#!/bin/bash
/etc/init.d/networking restart
ifconfig eth0 down
ifconfig eth0 up 172.16.10.1/24
route add default gw 172.16.10.254
printf "search netlab.fe.up.pt\nnameserver 172.16.1.1\n" > /etc/resolv.conf
