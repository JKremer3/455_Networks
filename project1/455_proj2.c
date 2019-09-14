#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/ether.h>
#include <linux/if_packet.h>
#include <linux/if.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/udp.h>

#define BUF_SIZ		65536
#define SEND 0
#define RECV 1

/*
struct  ethhdr {
	unsigned char h_dest[ETH_ALEN]; // destination eth addr
	unsigned char h_source[ETH_ALEN]; // source eth addr
	_be16		  h_proto;            //packet type ID field
}
*/

void send_message(char *interfaceName)
{
	int sock_send;
	struct ifreq if_config;
	char *interfaceHolder, *dataBuffer;
	struct ethhdr *ethernetHeader;
	struct iphdr *ipHeader;
	int totalSize; 

	sock_send = socket(AF_PACKET, SOCK_RAW, IPPROTO_RAW);
	if(sock_send < 0)
	{
		printf("Error: Socket\n");
		return;
	}

	interfaceHolder = (char *) malloc(sizeof(interfaceName));
	strcpy(interfaceHolder, interfaceName);

	memset(&if_config, 0, sizeof(if_config));
	strncpy(if_config.ifr_name, interfaceHolder, IFNAMSIZ-1);
	free(interfaceHolder);

	//Get interface index
	if((ioctl(sock_send, SIOCGIFINDEX, &if_config))< 0)
	{
		printf("ERROR: ioctl failure; SIOCGIFINDEX\n");
		return;
	}
	
	//Get MAC address of interface
	if((ioctl(sock_send, SIOCGIFHWADDR, &if_config))< 0)
	{
		printf("ERROR: ioctl failure; SIOCGIFHWADDR\n");
		return;
	}

	//Get IP Address of interface
	if((ioctl(sock_send, SIOCGIFADDR, &if_config))< 0)
	{
		printf("ERROR: ioctl failure; SIOCGIFADDR\n");
		return;
	}

	dataBuffer = (unsigned char *)malloc(64);
	memset(dataBuffer, 0 , 64);
	ethernetHeader = (struct ethhdr *)(dataBuffer);

	//set the ethernetHeader hardware source (MAC Address)
	for (int i = 0; i < ETH_ALEN; i++)
		ethernetHeader->h_source[i] = (unsigned char)(if_config.ifr_hwaddr.sa_data[i]);
	
	//fill destination hardware dest !--- Placeholder ---!
	for(int i = 0; i < ETH_ALEN; i++)
		ethernetHeader->h_dest[i] = 0x00;

	//Denotes next header as IP Header
	ethernetHeader->h_proto = htons(ETH_P_IP);
	totalSize = sizeof(struct ethhdr);
	
	//set all IP Header fields
	ipHeader = (struct iphdr*)(dataBuffer + sizeof(struct ethhdr));
	ipHeader->ihl = 5;
	ipHeader->version = 4;
	ipHeader->tos = 16;
	ipHeader->id = htons(10201);
	ipHeader->ttl = 64;
	ipHeader->protocol = 17;
	ipHeader->saddr = inet_addr(inet_ntoa((((struct sockaddr_in *)&(if_config.ifr_addr))->sin_addr)));
	//iph->daddr = inet_addr(destination_ip); // put destination IP address
}

void recv_message()
{
	//Do something here
	int sock_rec;
	unsigned char *buffer = (unsigned char *) malloc(BUF_SIZ);
	struct sockaddr sockAddress;
	int sockAddress_len = sizeof(struct sockaddr), buffLen = 0; 
	int ipHeaderLen, ethHeaderSize, udpHeaderSize;

	//open a raw socket to receive all packet
	sock_rec = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_IP));
	if( sock_rec < 0)
	{
		printf("Err: Bad Socket\n");
		return;
	}

	printf("Waiting for Message\n");
	while(buffLen == 0)
	{
		buffLen = recvfrom(sock_rec, buffer, BUF_SIZ, 0, &sockAddress, (socklen_t *) &sockAddress_len);
	}

	printf("Message Received\n");

	//store ethernet header
	struct ethhdr *ether = (struct ethhdr *) (buffer);
	ethHeaderSize = sizeof(struct ethhdr);
	//store IP header, and grab the length
	struct iphdr *ip = (struct iphdr *)(buffer + ethHeaderSize);
	ipHeaderLen = ip->ihl * 4;
	//Store UDP header, and store length
	struct udphdr *udp = (struct udphdr *)(buffer + ipHeaderLen + ethHeaderSize);
	udpHeaderSize = sizeof(struct udphdr);

	unsigned char *message = (buffer + ipHeaderLen + ethHeaderSize + udpHeaderSize);
	printf("received message: %s\n", message);
	free(buffer);
}

int main(int argc, char *argv[])
{
	int mode;
	char hw_addr[6];
	char interfaceName[IFNAMSIZ];
	char buf[BUF_SIZ];
	struct sockaddr_ll sk_addr;
	int sk_addr_size = sizeof(struct sockaddr_ll);
	memset(buf, 0, BUF_SIZ);

	int correct=0;
	if (argc > 1){
		


		if(strncmp(argv[1],"Send", 4)==0){
			if (argc == 5){
				mode=SEND; 
				sscanf(argv[3], "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", &hw_addr[0], &hw_addr[1], &hw_addr[2], &hw_addr[3], &hw_addr[4], &hw_addr[5]);
				strncpy(buf, argv[4], BUF_SIZ);
				correct=1;
				printf("  buf: %s\n", buf);
			}
		}
		else if(strncmp(argv[1],"Recv", 4)==0){
			if (argc == 3){
				mode=RECV;
				correct=1;
			}
		}
		strncpy(interfaceName, argv[2], IFNAMSIZ);
	 }
	 if(!correct){
		fprintf(stderr, "./455_proj2 Send <InterfaceName>  <DestHWAddr> <Message>\n");
		fprintf(stderr, "./455_proj2 Recv <InterfaceName>\n");
		exit(1);
	 }

	//Do something here

	if(mode == SEND){
		send_message(interfaceName);
	}
	else if (mode == RECV){
		
		recv_message();
	}

	return 0;
}

