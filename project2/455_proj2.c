#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/time.h>

#include <asm/types.h>

#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#include <netdb.h>
#include <netinet/ether.h>
#include <net/if.h>
#include <arpa/inet.h>

#include <linux/if_packet.h>
#include <linux/if_ether.h>

#define PROTO_ARP 0x0806
#define ETH2_HEADER_LEN 14
#define HW_TYPE 1
#define PROTOCOL_TYPE 0x800
#define MAC_LENGTH 6
#define IPV4_LENGTH 4
#define arpHolderUEST 0x01
#define ARP_REPLY 0x02
#define BUF_SIZE 60

//adjusted arp_hdr struct for readability
typedef struct arp_hdr
{
  unsigned short hardware_type;			//ar_hrd
  unsigned short protocol_type;			//ar_pro
  unsigned char hardware_len;			//ar_hln
  unsigned char  protocol_len;			//ar_pln
  unsigned short opcode;				//ar_op
  unsigned char sender_mac[MAC_LENGTH];	//ar_sha
  unsigned char sender_ip[IPV4_LENGTH];	//ar_sip
  unsigned char target_mac[MAC_LENGTH];	//ar_tha
  unsigned char target_ip[IPV4_LENGTH];	//ar_tip
}arp_hdr;

int process_arp(char *interfaceName, char* ipAddress)
{
	int status, response_len, bytes, sock;
	char *interfaceHolder, *ipHolder, *target;
	unsigned char src_ip_holder[IPV4_LENGTH];
	unsigned char dest_ip_holder[IPV4_LENGTH];
	unsigned char buffer[BUF_SIZE];
	struct arp_hdr *arpHolder 	= (struct arp_hdr *)(buffer+ETH2_HEADER_LEN);
  	struct arp_hdr *arpResponse = (struct arp_hdr *)(buffer+ETH2_HEADER_LEN); 
	struct ethhdr *send_req = (struct ethhdr*)buffer;
	struct ethhdr *recv_response = (struct ethhdr*)buffer;
	struct sockaddr_in *ipv4;
	struct sockaddr_ll deviceSock;
	struct ifreq if_MAC, if_ip, if_ind;
	struct addrinfo *res;
	
	interfaceHolder = (char*) malloc(sizeof(char) * strlen(interfaceName));
	ipHolder = (char*) malloc(sizeof(char) * strlen(ipAddress));
	memset(interfaceHolder, 0, sizeof(interfaceHolder));
	memset(ipHolder, 0, sizeof(ipHolder));
	strcpy(interfaceHolder, interfaceName);
	strcpy(ipHolder, ipAddress);
	
	memset(buffer, 0, sizeof(buffer));
	//attempt to get socket descriptor to look up local interface
	if((sock = socket (AF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) < 0)
	{
		printf("Socket() failed to open\n");
		free(ipHolder);
		free(interfaceHolder);
		return 0;
	}
	close(sock);
	sock = socket (AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));

	//Get Source MAC
	memset(&if_MAC, 0, sizeof(struct ifreq));
	strncpy(if_MAC.ifr_name, interfaceHolder, IFNAMSIZ-1);
	printf("Interface for MAC: %s\n", if_MAC.ifr_name);
	//Get MAC address of interface
	if((ioctl(sock, SIOCGIFHWADDR, &if_MAC))< 0)
	{
		printf("ERROR: ioctl failure; SIOCGIFHWADDR\n");
		free(ipHolder);
		free(interfaceHolder);
		return 0;
	}
	//Get ethernet interface i 
	memset(&if_ip, 0, sizeof(struct ifreq));
	strncpy(if_ip.ifr_name, interfaceHolder, IFNAMSIZ-1);
	//Get interface i
	if((ioctl(sock, SIOCGIFADDR, &if_ip)< 0))
	{
		printf("ERROR: ioctl failure; SIOCGIFi\n");
		free(ipHolder);
		free(interfaceHolder);
		return 0;
	}

	memset(&if_ind, 0, sizeof(struct ifreq));
	strncpy(if_ind.ifr_name, interfaceHolder, IFNAMSIZ-1);
	//printf("Interface for MAC: %s\n", if_ind.ifr_name);
	//Get MAC address of interface
	if((ioctl(sock, SIOCGIFINDEX, &if_ind))< 0)
	{
		printf("ERROR: ioctl failure; SIOCGIFINDEX\n");
		free(ipHolder);
		free(interfaceHolder);
		return 0;
	}
	//close(sock);
	
	printf("Source MAC: ");
	for(int i = 0; i < 6; i++)
	{
		arpHolder->sender_mac[i] = (unsigned char)(if_MAC.ifr_hwaddr.sa_data[i]);
		arpHolder->target_mac[i] = 0x00;
		send_req->h_source[i] 	 = (unsigned char)(if_MAC.ifr_hwaddr.sa_data[i]);
		send_req->h_dest[i]		 = 0xff; //spamming that packet
		printf("%02x:", arpHolder->sender_mac[i]);
	}
	printf("\n");

	printf("Source IP: %s\n", inet_ntoa(((struct sockaddr_in *)&if_ip.ifr_addr)->sin_addr));
	strcpy(src_ip_holder, inet_ntoa(((struct sockaddr_in *)&if_ip.ifr_addr)->sin_addr));
	if((inet_pton(AF_INET,src_ip_holder, arpHolder->sender_ip)) != 1)
	{
		printf("inet_pton() failed\n");
		free(ipHolder);
		free(interfaceHolder);
		return 0;
	}
	printf("%s", arpHolder->sender_ip);
	printf("\n");

	//Finding interface i and name, storing in deviceSock
	memset(&deviceSock, 0, sizeof(deviceSock));
	if((deviceSock.sll_ifindex = if_nametoindex(interfaceHolder)) < 0)
	{
		printf("ERROR: failed to set sll_ifindex\n");
		free(ipHolder);
		free(interfaceHolder);
		return 0;
	}

	strcpy(dest_ip_holder, ipHolder);
	if((getaddrinfo(dest_ip_holder, NULL, NULL, &res)) != 0)
	{
		printf("Error: getaddrinfo()\n");
		free(ipHolder);
		free(interfaceHolder);
		return 0;
	}
	ipv4 = (struct  sockaddr_in*) res->ai_addr; 
	memcpy(arpHolder->target_ip, &ipv4->sin_addr, sizeof(unsigned char) * 4);

	//set sockaddr_ll description
	deviceSock.sll_family 	= AF_PACKET;
  	deviceSock.sll_protocol = htons(ETH_P_ARP);
	deviceSock.sll_hatype 	= htons(ARPHRD_ETHER);
	deviceSock.sll_pkttype 	= (PACKET_BROADCAST);
	deviceSock.sll_halen 	= MAC_LENGTH;
	deviceSock.sll_addr[6] 	= 0x00;
	deviceSock.sll_addr[7] 	= 0x00;

	/* Setting protocol of the packet */
	send_req->h_proto = htons(ETH_P_ARP);

	/* Creating ARP request */
	arpHolder->hardware_type = htons(HW_TYPE);
	arpHolder->protocol_type = htons(ETH_P_IP);
	arpHolder->hardware_len = MAC_LENGTH;
	arpHolder->protocol_len =IPV4_LENGTH;
	arpHolder->opcode = htons(arpHolderUEST);
	
	//buffer[32] = 0x00;

	if((status = sendto(sock, buffer, 42, 0, (struct sockaddr*)&deviceSock, sizeof(deviceSock))) == -1)
	{
		printf("ERROR: failed to send\n");
		free(ipHolder);
		free(interfaceHolder);
		return 0;
	}
	else
	{
		printf("Sent ARP Request\n");
	}
	printf("\n\t");
  	memset(buffer,0x00,60);

	while(1)
  {
	response_len = recvfrom(sock, buffer, BUF_SIZE, 0, NULL, NULL);
	if (response_len == -1)
	{
			perror("recvfrom():");
			exit(1);
	}
	if(htons(recv_response->h_proto) == PROTO_ARP)
	{
			//if( arpResponse->opcode == ARP_REPLY )
			printf(" RECEIVED ARP RESP len=%d \n",response_len);
			printf(" Sender IP :");
			for(int i=0;i<4;i++)
				printf("%u.",(unsigned int)arpResponse->sender_ip[i]);

			printf("\n Sender MAC :");
			for(int i=0;i<6;i++)
				printf(" %02X:",arpResponse->sender_mac[i]);

			printf("\nReceiver  IP :");
			for(int i=0;i<4;i++)
				printf(" %u.",arpResponse->target_ip[i]);

			printf("\n Self MAC :");
			for(int i=0;i<6;i++)
				printf(" %02X:",arpResponse->target_mac[i]);

			printf("\n  :");

			break;
	}
  }

  return 0;
}

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
	 
	process_arp(interfaceName, ipAddress);

	free(ipAddress);
	return 0;
}

