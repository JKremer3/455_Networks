import sys
import socket
from scapy.all import *
from netaddr import *

#broadcast = "ff:ff:ff:ff:ff:ff"

def getSystemInformation():
    localMAC = get_if_hwaddr(conf.iface) 
    localIP = ARP().psrc

    print("Local MAC: {}".format(localMAC))
    print("Local IP : {}".format(localIP)) 

def createEthernetHeader(destinationAddress = None):
    ethernetHeader = None

    #If no destination address is provided, 
    #build the ethernetHeader for ARP
    if (destinationAddress == None):
        ethernetHeader = Ether() #defaults destination to broadcast
        ethernetHeader.type = 0x0806 #ARP type
    else:
        ethernetHeader = Ether(dst = destinationAddress)
        ethernetHeader.type = 0x0800 #IPv4 type

    return ethernetHeader

def createARPheader(destinationIP):
    arpHeader = ARP()
    arpHeader.pdst = destinationIP

    return arpHeader

def processARP(packet):
    arpRes = srp1(packet, timeout = 2)

    arpRes.show()
    if(arpRes[0][Ether].dst == get_if_hwaddr(conf.iface)):
        print("ARP received")
        arpRes[0].show()
        return arpRes[0][Ether].dst
    else:
        print("Failed to Receive ARP")

def sendIPpacket(interface, destinationIP, routerIP, message):
    print("Interface: {}".format(interface))
    print("destIP   : {}".format(destinationIP))
    print("routerIP : {}".format(routerIP))
    print("message  : {}".format(message))

    raw_message = Raw(load = message)
    
    #first create and send ARP to get HW address
    ethhdrArp = createEthernetHeader()
    arphdr    = createARPheader(destinationIP)
    arpPacket = ethhdrArp/arphdr #put together the arp packet

    #send the arp to get the destination MAC
    destMAC   = processARP(arpPacket)

    #with the destination MAC, start building IP packet
    ethhdrIP  = createEthernetHeader(destMAC)
    iphdr     = IP(dst = destinationIP)
    iphdr.ttl = 6

    #construct the full packet
    fullPack  = ethhdrIP/iphdr
    fullPack.add_payload(raw_message)
    fullPack.show()

    #send the packet
    sendp(fullPack)
    
def recvIPpacket(interface):
    print("Interface: {}".format(interface))
    while(1):
        recievedPack = sniff()
        recievedPack.show()
        if(recievedPack[0].haslayer("Raw") and recievedPack[0][Ethernet].dst == get_if_hwaddr(conf.iface)):
            #recievedPack.show()
            print("message: {}".format(recievedPack[0][raw].load))
            return

def runDebug(param):
    if(param == "ARP"):
        print("Test sending an ARP packet")
        ethhdr = createEthernetHeader()
        arphdr = createARPheader("10.0.0.2")
        packet = ethhdr/arphdr
        ethhdr.show()
        arphdr.show()
        print("\nFull Packet:")
        packet.show()
        processARP(packet)
    elif(param == "Send"):
        print("Sending Packet on local network") 
        sendIPpacket("h1-eth0","10.0.0.2", "10.0.0.1", "Hello World")
    elif(param == "Recv"):
        recvIPpacket("h2-eth0")
    else:
        print("Error: invalid debug command")


if __name__ == "__main__":
    global debug
    debug = 0
    if(len(sys.argv) > 1 and sys.argv[1] == "debug"):
        debug = 1
    
    if(len(sys.argv) == 6 and sys.argv[1] == "Send" and debug == 0):
        interfaceHolder = sys.argv[2]
        destinationIP   = sys.argv[3]
        routerIP        = sys.argv[4]
        message         = sys.argv[5]
        sendIPpacket(interfaceHolder, destinationIP, routerIP, message)
    elif(len(sys.argv) == 3 and sys.argv[1] == "Recv" and debug == 0):
        interfaceHolder = sys.argv[2]
        recvIPpacket(interfaceHolder)
    elif(debug == 1):
        if(len(sys.argv) == 2):
            print("Error: No Parameter passed in\nPlease specify <ARP>, <Send>, or <Recv>")
        else:
            runDebug(sys.argv[2])
    else:
        print("Usage:  python3 ./455_proj3.py Send <InterfaceName> <DestIP> <Router IP> <Message>\n\tpython3 ./455_proj3.py Send <InterfaceName>") 

    





