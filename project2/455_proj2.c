#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/ether.h>
#include <linux/if_packet.h>
#include <linux/if.h>
#include <net/if_arp.h>

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

void send_recv_address(char *interfaceName, char* ipAddres)
{
	int sock;
	int textLength = 0, buffLength, i, isSent; 
	
	char dataBuffer[BUF_SIZ];
	char sourceMAC[6];
	char *interfaceHolder; 
	char *ethHead = dataBuffer; 
	char *arpHead = dataBuffer +14;
	
	struct ethhdr *ethernetHeader = (struct ethhdr *)dataBuffer;
	struct arphdr *arpHeader;
	struct sockaddr_ll socketAddr;
	struct ifreq if_ind, if_MAC;

	//Open the Socket as a Raw Socket
	sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if(sock < 0)
	{
		printf("Error: Socket\n");
		return;
	}

	printf("Succesfully Opened Socket");

	interfaceHolder = (char *) malloc(sizeof(interfaceName));
	memset(interfaceHolder, 0, sizeof(interfaceName));
	strcpy(interfaceHolder, interfaceName);

	//Get ethernet interface index 
	memset(&if_ind, 0, sizeof(struct ifreq));
	strncpy(if_ind.ifr_name, interfaceHolder, IFNAMSIZ-1);
	if((ioctl(sock, SIOCGIFINDEX, &if_ind)< 0))
	{
		printf("ERROR: ioctl failure; SIOCGIFINDEX\n");
		return;
	}
	
	//Get MAC address of interface
	memset(&if_MAC, 0, sizeof(struct ifreq));
	strncpy(if_MAC.ifr_name, interfaceHolder, IFNAMSIZ-1);
	if((ioctl(sock, SIOCGIFHWADDR, &if_MAC))< 0)
	{
		printf("ERROR: ioctl failure; SIOCGIFHWADDR\n");
		return;
	}

	memset(dataBuffer,0, BUF_SIZ);
	memset(sourceMAC, 0 , 6);

	//store the source MAC Address
	for (i = 0; i < ETH_ALEN; i++)
		sourceMAC[i] = (unsigned char)(if_MAC.ifr_hwaddr.sa_data[i]);

	//Set socketAddr 
	socketAddr.sll_family = PF_PACKET;
	socketAddr.sll_protocol  = htons(ETH_P_ARP);
	socketAddr.sll_ifindex = if_ind.ifr_ifindex;
	socketAddr.sll_hatype = ARPHRD_ETHER;
	socketAddr.sll_pkttype = PACKET_OTHERHOST;
	socketAddr.sll_halen = 0;
	socketAddr.sll_addr[6] = 0x00;
	socketAddr.sll_addr[7] = 0x00;

	/*if(sendto(sock_send, dataBuffer, textLength, 0, (struct sockaddr*)&socketAddr, sizeof(struct sockaddr_ll)) < 0)
	{
		printf("Message Failed to Send\n");
	} */
	while(1)
	{
		buffLength = recvfrom(sock, dataBuffer, BUF_SIZ, 0, NULL, NULL);
		if(buffLength == -1)
		{
			perror("recvfrom:");
		}

		
		if(ntohs(ethernetHeader->h_proto == ETH_P_ARP)) //check if the packet type is ARP
		{
			char buffer_ARP_DHA[6], buffer_ARP_DPA[4];
			arpHeader = (struct arphdr*)arpHead;

		}

		if(sendto(sock, dataBuffer, BUF_SIZ, 0, (struct sockaddr*)&socketAddr, sizeof(struct sockaddr_ll)) < 0)
		{
			printf("Message Failed to Send\n");
		} 
	}

	printf("message sent!\n");
	free(interfaceHolder);
}

/*void recv_message(char* interface)
{
	int sock_rec;
	unsigned char *buffer = (unsigned char *) malloc(BUF_SIZ), *interfaceHolder;
	struct  ifreq if_MAC;
	struct sockaddr sockAddress;
	int sockAddress_len = sizeof(struct sockaddr), buffCheck = 0; 
	int ethHeaderSize, ethCheck = 1;

	//open a raw socket to receive all packet
	sock_rec = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_IP));
	
	if( sock_rec < 0)
	{
		printf("Err: Bad Socket\n");
		return;
	}

	interfaceHolder = (char *) malloc(sizeof(interface));
	memset(interfaceHolder, 0, sizeof(interface));
	strcpy(interfaceHolder, interface);

	memset(&if_MAC, 0, sizeof(struct ifreq));
	strncpy(if_MAC.ifr_name, interfaceHolder, IFNAMSIZ-1);
	//Get MAC address of interface
	if((ioctl(sock_rec, SIOCGIFHWADDR, &if_MAC))< 0)
	{
		printf("ERROR: ioctl failure; SIOCGIFHWADDR\n");
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
	unsigned char *message = (buffer + ethHeaderSize );
	
	for (int i = 0; i < 6; i++)
		if(ether->h_dest[i] != (unsigned char)(if_MAC.ifr_hwaddr.sa_data[i]))
			ethCheck = 0;

	if(ethCheck != 0)
	{
		//Read the message from the buffer
		
		printf("Message Received from MAC: ");
		for(int i = 0; i < 6; i++)
			printf("%hhx:", ether->h_source[i]); 
		printf("\n Type: %x\n", ether->h_proto);
		printf("received message: %s\n", message);
	}
	else
	{
		printf("Error: Received message was meant for different address\n");
	}
	
	free(buffer);
	free(interfaceHolder);
}*/

int main(int argc, char *argv[])
{
	//int mode;
	int hw_addr[6];
	char interfaceName[IFNAMSIZ];
	//char buf[BUF_SIZ];
	char* ipAddress;
	struct sockaddr_ll sk_addr;
	int sk_addr_size = sizeof(struct sockaddr_ll);
	//memset(buf, 0, BUF_SIZ);

	int correct=0;
	if (argc == 3){	
		printf("Interface: %s\n", argv[1]);
		strncpy(interfaceName, argv[1], IFNAMSIZ);
		ipAddress = strdup(argv[2]);
		correct=1;
	}
	else
	{
		fprintf(stderr, "./455_proj2 <InterfaceName>  <DestIPAddress> \n");
		exit(1);
	}
	 
	send_recv_address(interfaceName, ipAddress);
	//Do something here

	free(ipAddress);
	return 0;
}

