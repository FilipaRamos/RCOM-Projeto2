#!/bin/bash
/etc/init.d/networking restart
ifconfig eth0 down
ifconfig eth0 up 172.16.11.1/24
route add default gw 172.16.11.254
route add -net 172.16.10.0/24 gw 172.16.11.253
echo 0 > /proc/sys/net/ipv4/icmp_echo_ignore_broadcasts
printf "search lixa.netlab.fe.up.pt\nnameserver 172.16.1.1\n" > /etc/resolv.conf
