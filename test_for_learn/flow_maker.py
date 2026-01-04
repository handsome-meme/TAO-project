#!/usr/bin/python

from scapy.all import *

underlay = Ether(src="00:00:00:00:00:01", dst="22:d0:2e:3d:42:c2")/IP(dst='1.0.0.11',src='1.0.0.1')
while True:
    i = random.randint(1, 10000000)
    if i <= 6000000:
        j = random.randint(1, 5000) 
    elif i <= 8000000:
        j = random.randint(5001, 15000)
    elif i <= 9000000:
        j = random.randint(15001, 35000)
    elif i <= 9800000:
        j = random.randint(35001, 65000)
    else :    
        j = random.randint(65001, 100000)
    d = "0x" + str(j)
    sr = "0x1" + str(j)
    p = underlay/UDP(dport=4789,sport=(j%65535))/VXLAN(vni=j)/Ether(src="11:22:33:44:55:66", dst="11:22:33:44:55:67")/IP(dst=d,src=sr)    
    sendp(p, iface="veth1",count=1) 

# 造包期间 不要使用tofino 设备不要断电
# tcpdump -i enp216s0f1 udp port 4789 -w data_make.pcap

# ratio
# 60%   80%   90%   98%   100%
# 5%    15%   35%   65%   100%

# precision 
# 