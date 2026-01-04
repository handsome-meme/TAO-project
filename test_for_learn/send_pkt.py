#!/usr/bin/python

from itertools import count
from multiprocessing.connection import wait
import os
import random
import sys
from time import sleep

if os.getuid() !=0:
    print """
ERROR: This script requires root privileges. 
       Use 'sudo' to run it.
"""
    quit()

from scapy.all import *

# try:
#     ip_dst = sys.argv[1]
# except:
#     ip_dst = "192.168.1.2"

# try:
#     iface = sys.argv[2]
# except:
#     iface="veth0"

# underlay = Ether(src="00:00:00:00:00:01", dst="00:00:00:00:00:11")/IP(dst='1.0.0.11',src='1.0.0.1', options=False)
# vxlanh = UDP(dport=4789,sport=12345)/VXLAN(vni=1)/Ether(src="11:22:33:44:55:66", dst="11:22:33:44:55:67")/IP(dst='10.0.0.11',src='10.0.0.1', options=False)


underlay = Ether(src="00:00:00:00:00:01", dst="22:d0:2e:3d:42:c2")/IP(dst='1.0.0.11',src='1.0.0.1')
vxlanh1 = UDP(dport=4789,sport=12345)/VXLAN(vni=1)/Ether(src="11:22:33:44:55:66", dst="11:22:33:44:55:67")/IP(dst='10.0.0.11',src='10.0.0.1')
vxlanh2 = UDP(dport=4789,sport=12345)/VXLAN(vni=2)/Ether(src="11:22:33:44:55:66", dst="11:22:33:44:55:67")/IP(dst='10.0.0.12',src='10.0.0.2')
vxlanh3 = UDP(dport=4789,sport=12345)/VXLAN(vni=3)/Ether(src="11:22:33:44:55:66", dst="11:22:33:44:55:67")/IP(dst='10.0.0.13',src='10.0.0.3')
vxlanh4 = UDP(dport=4789,sport=12345)/VXLAN(vni=4)/Ether(src="11:22:33:44:55:66", dst="11:22:33:44:55:67")/IP(dst='10.0.0.14',src='10.0.0.4')
vxlanh5 = UDP(dport=4789,sport=12345)/VXLAN(vni=5)/Ether(src="11:22:33:44:55:66", dst="11:22:33:44:55:67")/IP(dst='10.0.0.15',src='10.0.0.5')
vxlanh6 = UDP(dport=4789,sport=12345)/VXLAN(vni=6)/Ether(src="11:22:33:44:55:66", dst="11:22:33:44:55:67")/IP(dst='10.0.0.16',src='10.0.0.6')
vxlanh7 = UDP(dport=4789,sport=12345)/VXLAN(vni=7)/Ether(src="11:22:33:44:55:66", dst="11:22:33:44:55:67")/IP(dst='10.0.0.17',src='10.0.0.7')

p = underlay/vxlanh1
sendp(p, iface="veth1",count=1) 

underlay = Ether(src="00:00:00:00:00:01", dst="22:d0:2e:3d:42:c2")/IP(dst='1.0.0.11',src='1.0.0.1')
while True:
    i = random.randint(1,10)
    d = "10.0.0." + str(i)
    sr = "10.0.1." + str(i)
    p = underlay/UDP(dport=4789,sport=12345)/VXLAN(vni=i)/Ether(src="11:22:33:44:55:66", dst="11:22:33:44:55:67")/IP(dst=d,src=sr)

    j = random.randint(90,100)
    sendp(p, iface="veth1",count=j) 

    ii = random.randint(11,100)
    d = "10.0.0." + str(ii)
    sr = "10.0.1." + str(ii)
    pp = underlay/UDP(dport=4789,sport=12345)/VXLAN(vni=ii)/Ether(src="11:22:33:44:55:66", dst="11:22:33:44:55:67")/IP(dst=d,src=sr)
    
    j = random.randint(1,10)
    sendp(pp, iface="veth1",count=j)

count = 0
cnt = [0]*105
while count < 5000:
    count += 1
    i = random.randint(1,100)

    if i <= 80:
        j = random.randint(1,20) 
    else :
        j = random.randint(21,100)

    cnt[j] += 1
    d = "10.0.0." + str(j)
    sr = "10.0.1." + str(j)
    p = underlay/UDP(dport=4789,sport=12345)/VXLAN(vni=j)/Ether(src="11:22:33:44:55:66", dst="11:22:33:44:55:67")/IP(dst=d,src=sr)    
    sendp(p, iface="veth1",count=1) 

i = 0
while i < len(cnt):
    print("index=%d value=%d" % (i, cnt[i]))
    i += 1

count = 0
while count < 5000:
    count += 1
    i = random.randint(1,200)

    if i <= 80:
        j = random.randint(1,20) 
    else :
        j = random.randint(21,200)

    d = "10.0.0." + str(j)
    sr = "10.0.1." + str(j)
    p = underlay/UDP(dport=4789,sport=12345)/VXLAN(vni=j)/Ether(src="11:22:33:44:55:66", dst="11:22:33:44:55:67")/IP(dst=d,src=sr)    
    sendp(p, iface="veth1",count=1) 

# 10-60 10-20 80-20
count = 0
while count < 5000:
    count += 1
    i = random.randint(1,100)

    if i <= 70:
        j = random.randint(1,10) 
    elif i <= 85:
        j = random.randint(11,20)
    else :    
        j = random.randint(21,100)

    d = "10.0.0." + str(j)
    sr = "10.0.1." + str(j)
    p = underlay/UDP(dport=4789,sport=12345)/VXLAN(vni=j)/Ether(src="11:22:33:44:55:66", dst="11:22:33:44:55:67")/IP(dst=d,src=sr)    
    sendp(p, iface="veth1",count=1) 


p1 = underlay/vxlanh1
p2 = underlay/vxlanh2
p3 = underlay/vxlanh3
p4 = underlay/vxlanh4
p5 = underlay/vxlanh5
p6 = underlay/vxlanh6
p7 = underlay/vxlanh7
sendp(p1, iface="veth1",count=1) 

while True:
    i = random.randint(1,7)
    if i == 1:
        sendp(p7, iface="veth1",count=80)
    elif i == 2:
        sendp(p2, iface="veth1",count=2)
    elif i == 3:
        sendp(p3, iface="veth1",count=3)
    elif i == 4:
        sendp(p4, iface="veth1",count=5)
    elif i == 5:
        sendp(p5, iface="veth1",count=7)
    elif i == 6:
        sendp(p6, iface="veth1",count=1)
    else :
        sendp(p1, iface="veth1",count=2)

   



# p = Ether(dst="ca:f0:ab:64:f2:59")/IP()/UDP()

# 造包期间 不要使用tofino 设备不要断电
# tcpdump -i enp216s0f1 udp port 4789 -w data_make.pcap

underlay = Ether(src="00:00:00:00:00:01", dst="22:d0:2e:3d:42:c2")/IP(dst='1.0.0.11',src='1.0.0.1')
while True:
    i = random.randint(1, 10000000)
    if i <= 6000000:
        j = random.randint(1, 5000) 
    elif i <= 8000000:
        j = random.randint(500001, 15000)
    elif i <= 9000000:
        j = random.randint(1500001, 35000)
    elif i <= 9800000:
        j = random.randint(3500001, 65000)
    else :    
        j = random.randint(6500001, 100000)
    d = "0x" + str(j)
    sr = "0x1" + str(j)
    p = underlay/UDP(dport=4789,sport=12345)/VXLAN(vni=j)/Ether(src="11:22:33:44:55:66", dst="11:22:33:44:55:67")/IP(dst=d,src=sr)    
    sendp(p, iface="enp216s0f1",count=1) 

