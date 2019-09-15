#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/ether.h>
#include <linux/if_packet.h>
#include <linux/if.h>

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

void send_message(char *interfaceName, int hwAddress[], char message[])
{
	int sock_send;
	struct ifreq if_ind, if_MAC;
	char *interfaceHolder, dataBuffer[BUF_SIZ];
	struct ethhdr *ethernetHeader;
	struct sockaddr_ll socketAddr;
	int textLength = 0, i; 

	//Open the Socket as a Raw Socket
	sock_send = socket(AF_PACKET, SOCK_RAW, IPPROTO_RAW);
	if(sock_send < 0)
	{
		printf("Error: Socket\n");
		return;
	}

	interfaceHolder = (char *) malloc(sizeof(interfaceName));
	memset(interfaceHolder, 0, sizeof(interfaceName));
	strcpy(interfaceHolder, interfaceName);

	memset(&if_ind, 0, sizeof(struct ifreq));
	strncpy(if_ind.ifr_name, interfaceHolder, IFNAMSIZ-1);
	//Get interface index
	if((ioctl(sock_send, SIOCGIFINDEX, &if_ind)< 0))
	{
		printf("ERROR: ioctl failure; SIOCGIFINDEX\n");
		return;
	}
	
	memset(&if_MAC, 0, sizeof(struct ifreq));
	strncpy(if_MAC.ifr_name, interfaceHolder, IFNAMSIZ-1);
	//Get MAC address of interface
	if((ioctl(sock_send, SIOCGIFHWADDR, &if_MAC))< 0)
	{
		printf("ERROR: ioctl failure; SIOCGIFHWADDR\n");
		return;
	}

	memset(dataBuffer,0, BUF_SIZ);
	ethernetHeader = (struct ethhdr *)(dataBuffer);

	//set the ethernetHeader hardware source (MAC Address)
	for (i = 0; i < ETH_ALEN; i++)
		ethernetHeader->h_source[i] = (unsigned char)(if_MAC.ifr_hwaddr.sa_data[i]);
	
	//fill destination hardware dest 
	for( i = 0; i < ETH_ALEN; i++)
		ethernetHeader->h_dest[i] = hwAddress[i];

	//sets the etherType
	ethernetHeader->h_proto = htons(ETH_P_IP);
	
	textLength += sizeof(struct ethhdr);

	i = 0;
	while (message[i] != '\0')
	{
		dataBuffer[textLength++] = message[i];
		i++;
	}

	socketAddr.sll_ifindex = if_ind.ifr_ifindex; //network index
	socketAddr.sll_halen = ETH_ALEN; //address length
	
	for(i = 0; i <ETH_ALEN; i++) //fill in dest MAC
		socketAddr.sll_addr[i] = hwAddress[i];

	if(sendto(sock_send, dataBuffer, textLength, 0, (struct sockaddr*)&socketAddr, sizeof(struct sockaddr_ll)) < 0)
	{
		printf("Message Failed to Send\n");
	} 

	printf("message sent!\n");
	free(interfaceHolder);
}

void recv_message(char* interface)
{
	int sock_rec;
	unsigned char *buffer = (unsigned char *) malloc(BUF_SIZ);
	struct sockaddr sockAddress;
	int sockAddress_len = sizeof(struct sockaddr), buffCheck = 0; 
	int ethHeaderSize;

	//open a raw socket to receive all packet
	sock_rec = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_IP));
	if( sock_rec < 0)
	{
		printf("Err: Bad Socket\n");
		return;
	}

	printf("Waiting for Message\n");
	//loop here until a message is received
	while(buffCheck == 0)
	{
		buffCheck = recvfrom(sock_rec, buffer, BUF_SIZ, 0, &sockAddress, (socklen_t *) &sockAddress_len);
	}

	printf("Message Received\n");

	//store ethernet header
	struct ethhdr *ether = (struct ethhdr *) (buffer);
	ethHeaderSize = sizeof(struct ethhdr);

	//Read the message from the buffer
	unsigned char *message = (buffer + ethHeaderSize );
	printf("received message: %s\n", message);
	free(buffer);
}

int main(int argc, char *argv[])
{
	int mode;
	int hw_addr[6];
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
				printf("Interface: %s\n", argv[2]);
				sscanf(argv[3], "%02x:%02x:%02x:%02x:%02x:%02x", &hw_addr[0], &hw_addr[1], &hw_addr[2], &hw_addr[3], &hw_addr[4], &hw_addr[5]);
				printf("Destination MAC: %02x:%02x:%02x:%02x:%02x:%02x\n", hw_addr[0], hw_addr[1],hw_addr[2],hw_addr[3],hw_addr[4], hw_addr[5]);
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
		send_message(interfaceName, hw_addr, buf);
	}
	else if (mode == RECV){
		
		recv_message(interfaceName);
	}

	return 0;
}

