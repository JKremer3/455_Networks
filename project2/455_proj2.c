#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/ether.h>
#include <linux/if_packet.h>
#include <net/if_arp.h>
#include <net/if.h>

#define BUF_SIZ		65536
#define SEND 0
#define RECV 1


struct arp_hdr
{
	uint16_t		ar_hrd;
	uint16_t		ar_pro;
	unsigned char	ar_hln;
	unsigned char	ar_pln;
	uint16_t		ar_op;
	unsigned char	ar_sha[6];
	unsigned char	ar_sip[4];
	unsigned char	ar_tha[6];
	unsigned char	ar_tip[4];
}arp_hdr;

int send_arp_req(char *interfaceName, char* ipAddress)
{
	int status, frame_len, bytes, sock;
	char *interfaceCopy, *ipCopy, *target;
	struct arp_hdr arpHolder; 
	struct  sockaddr_in *ipv4;
	struct sockaddr_ll deviceSock;
	struct ifreq if_MAC, if_ind;
	
	interfaceCopy = (char*) malloc(sizeof(char) * strlen(interfaceName));
	ipCopy = (char*) malloc(sizeof(char) * strlen(ipAddress));
	strcpy(interfaceCopy, interfaceName);
	strcpy(ipCopy, ipAddress);
	
	//attempt to get socket descriptor to look up local interface
	if((sock = socket (AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0)
	{
		printf("Socket() failed to open\n");
		free(ipCopy);
		free(interfaceCopy);
		return 0;
	}
	close(sock);
	sock = socket (AF_INET, SOCK_RAW, IPPROTO_RAW);

	//Get Source MAC
	memset(&if_MAC, 0 , sizeof(if_MAC));
	strncpy(if_MAC.ifr_name, interfaceCopy, IF_NAMESIZE-1);
	printf("Interface for MAC: %s\n", if_MAC.ifr_name);
	if((ioctl(sock, SIOCGIFHWADDR, &if_MAC)) < 0);
	{
		printf("ioctl() failed to get source MAC\n");
		free(ipCopy);
		free(interfaceCopy);
		return 0;
	}
	
	//Get ethernet interface index 
	memset(&if_ind, 0, sizeof(struct ifreq));
	strncpy(if_ind.ifr_name, interfaceCopy, IF_NAMESIZE-1);
	if((ioctl(sock, SIOCGIFADDR, &if_ind)) < 0)
	{
		printf("ERROR: ioctl failure; SIOCGIFADDR\n");
		free(ipCopy);
		free(interfaceCopy);
		return 0;
	}
	//close(sock);
	
	printf("Source MAC:");
	for(int i = 0; i < 6; i++)
	{
		printf("%02x:", if_MAC.ifr_hwaddr.sa_data[i]);
		arpHolder.ar_sha[i] = (unsigned char)(if_MAC.ifr_hwaddr.sa_data[i]);
	}
	printf("\n");

	printf("Source IP:");
	for(int i = 0; i < 4; i++)
	{
		printf("%03x:", if_ind.ifr_addr.sa_data[i]);
		arpHolder.ar_sip[i] = (unsigned char)(if_ind.ifr_addr.sa_data[i]);
	}
	printf("\n");

	//Finding interface index and name, storing in deviceSock
	memset(&deviceSock, 0, sizeof(deviceSock));
	if((deviceSock.sll_ifindex = if_nametoindex(interfaceCopy)) < 0)
	{
		printf("ERROR: failed to set sll_ifindex\n");
		free(ipCopy);
		free(interfaceCopy);
		return 0;
	}
}

/*void send_recv_address(char *interfaceName, char* ipAddress)
{
	int sock;
	int textLength = 0, buffLength, i, isSent; 
	
	char dataBuffer[BUF_SIZ];
	char sourceMAC[6];
	char *interfaceHolder; 
	char *ethHead = dataBuffer; 
	char *arpHead = dataBuffer + 14;
	
	struct ethhdr *ethernetHeader = (struct ethhdr *)dataBuffer;
	struct arp_hdr *arpHeader;
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

	
	while(1)
	{
		buffLength = recvfrom(sock, dataBuffer, BUF_SIZ, 0, NULL, NULL);
		if(buffLength == -1)
		{
			printf("Failed to receive a message\n");
		}

		
		if(ntohs(ethernetHeader->h_proto == ETH_P_ARP)) //check if the packet type is ARP
		{
			char buffer_ARP_DHA[6], buffer_ARP_DPA[4];
			arpHeader = (struct arp_hdr*)arpHead;

			if(ntohs(arpHeader->arp_op) != ARPOP_REQUEST)
			{
				printf("Error: not an ARP request\n");
				break;
			}

			printf("Response MAC: %02x:%02x:%02x:%02x:%02x:%02x", arpHeader->arp_sha[0], arpHeader->arp_sha[1], arpHeader->arp_sha[2], arpHeader->arp_sha[3],arpHeader->arp_sha[4], arpHeader->arp_sha[5]);
			printf("Response IP: %02d:%02d:%02d:%02d\n", arpHeader->arp_spa[0], arpHeader->arp_spa[1], arpHeader->arp_spa[2], arpHeader->arp_spa[3]);
		}

		if(sendto(sock, dataBuffer, BUF_SIZ, 0, (struct sockaddr*)&socketAddr, sizeof(struct sockaddr_ll)) < 0)
		{
			printf("Message Failed to Send\n");
		} 
	}

	printf("message sent!\n");
}	*/

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
	 
	send_arp_req(interfaceName, ipAddress);
	//Do something here

	free(ipAddress);
	return 0;
}

