#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/if_ether.h>
#include <net/if_arp.h>
#include <net/if.h>
#include <net/ethernet.h>
#include <netpacket/packet.h>
#include <netinet/ether.h>
#include <string.h>



// #define INTERFAXENAME "veth4"
//子函数，用于显示源目的IP地址
void showip(unsigned char *buffer)
{
	struct iphdr *iph;
	char sipaddr[INET_ADDRSTRLEN] = "";
	char dipaddr[INET_ADDRSTRLEN] = "";
	iph = (struct iphdr*)(buffer+sizeof(struct ethhdr));
	inet_ntop(AF_INET,&iph->saddr,sipaddr,INET_ADDRSTRLEN);
	inet_ntop(AF_INET,&iph->daddr,dipaddr,INET_ADDRSTRLEN);
	printf("IP: %s >> %s\n",sipaddr,dipaddr);
}

int main(int argc, char **argv)
{
	int sock, n;
	unsigned char buffer[1024];
	struct ethhdr *eth;
	struct iphdr *iph;
	struct ifreq ethreq;
	char type[5]="";
    char et[24]="";
	uint16_t *port=NULL;

	//创建原始套接字
	if(0>(sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL))))
	{
		perror("socket");
		exit(1);
	}

    // struct ifreq interface;
    // strncpy(interface.ifr_ifrn.ifrn_name, "veth0", strlen("veth0"));
    // if (setsockopt(sock, SOL_SOCKET, SO_BINDTODEVICE, (char *)&interface, sizeof(interface))  < 0) {
    //        perror("SO_BINDTODEVICE failed");
    // }

	//接收数据包

	while(1)
	{
		bzero(buffer,sizeof(buffer));
		n=recvfrom(sock,buffer,1024,0,NULL,NULL);

        eth = (struct ethhdr*)buffer;
       // printf("%s \n",eth->h_dest);
        sprintf(et,"%02x:%02x:%02x:%02x:%02x:%02x",eth->h_dest[0],eth->h_dest[1],eth->h_dest[2],eth->h_dest[3],eth->h_dest[4],eth->h_dest[5]);
	//printf("%s\n",et);
        if(0 == strcmp(et,"22:d0:2e:3d:42:c2")){
            //分析数据包类型
            sprintf(type,"%02x%02x",buffer[12],buffer[13]);
            //0800IP数据包

            if(0 == strcmp(type,"0800"))
            {
                printf("============IP===============\n");
                //TCP还是UDP
                if(0x06 == buffer[23])
                {
                    printf("TCP\n");
                }
                else if(0x11 == buffer[23])
                {
                    printf("UDP\n");
                }
                //端口
                port= (uint16_t *)(buffer+34);
                printf("Port:%d >> ",ntohs(*port));
                port= (uint16_t *)(buffer+36);
                printf("%d\n",ntohs(*port));
                //输出IP地址
                showip(buffer);
            }
            else if(0 == strcmp(type,"0806"))
            {
                printf("============ARP===============\n");
                showip(buffer);
            }
            else if(0 == strcmp(type,"8035"))
            {
                printf("============RARP===============\n");
            }
            else
            {
                printf("============%s===============\n",type);
            }
            
            //显示MAC地址
            printf("MAC: %02X:%02X:%02X:%02X:%02X:%02X >> %02X:%02X:%02X:%02X:%02X:%02X \n",eth->h_source[0],eth->h_source[1],eth->h_source[2],eth->h_source[3],eth->h_source[4],eth->h_source[5],eth->h_dest[0],eth->h_dest[1],eth->h_dest[2],eth->h_dest[3],eth->h_dest[4],eth->h_dest[5]);		
        }
		
	}
}
