#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/ether.h>
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

void send_message(){


}

void recv_message(){
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
		send_message();
	}
	else if (mode == RECV){
		
		recv_message();
	}

	return 0;
}

