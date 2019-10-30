import sys
import socket
import netifaces
from scapy.all import *
from netaddr import *

#broadcast = "ff:ff:ff:ff:ff:ff"

def sendEthernetMessage(message, destinationAddress = None):
    #create an ethernet packet, setting the destination address, and the packet type to IPv4
    ethernetHeader = Ether(dst = destinationAddress)
    ethernetHeader.type = 0x0800 #IPv4 type
    
    rawPayload = Raw(load=message)
    ethernetHeader.add_payload(rawPayload)
    
    if(debug):
        print("Ethernet Packet")
        ethernetHeader.show()
    sendp(ethernetHeader)

def sendARP(destinationIP):
    arpEthHeader = Ether(type = 0x806, dst = "ff:ff:ff:ff:ff:ff")
    arpHeader = ARP(pdst=destinationIP)
    arpPacket = arpEthHeader/arpHeader

    if(debug):
        arpPacket.show()
    
    response = srp1(arpPacket)

    try:
        if(response.haslayer(ARP)):
            print("Recieved ARP resonse")
            response[ARP].show()
            return response[ARP].hwsrc
    except:
        print("Response Timed Out")

def calculateNetmask(destinationIP):
    localInterface = conf.iface
    netInfo = netifaces.ifaddresses(localInterface)

    localIP = netInfo[netifaces.AF_INET][0]['addr']
    localNetmask = netInfo[netifaces.AF_INET][0]['netmask']

    if(IPNetwork(localIP+'/'+localNetmask) == IPNetwork(destinationIP + '/' + localNetmask)):
        return True
    else:
        return False


def sendIPpacket(interface, destinationIP, routerIP, message):
    print("Interface: {}".format(interface))
    print("destIP   : {}".format(destinationIP))
    print("routerIP : {}".format(routerIP))
    print("message  : {}".format(message))

    raw_message = Raw(load=message)
    
    #first check the netmask,
    #IF the location is in network, ARP the location directly
    #otherwise, ARP the router
    if(calculateNetmask(destinationIP)):
        destinationMAC = sendARP(destinationIP)
    else:
        destinationMAC = sendARP(routerIP)
    
    #then build the packet, sending to the router IP
    ethHdr = Ether(dst = destinationMAC, type = 0x0800)
    ipHdr = IP(dst = destinationIP, ttl = 6, proto = 253)
    packet = ethHdr/ipHdr
    packet.add_payload(raw_message)
    packet.show()

    #clear default checksum and recalculate
    del packet[IP].chksum
    packet = packet.__class__(bytes(packet))
    print("Checksum calculation: {}".format(hex(packet[IP].chksum)))

    sendp(packet)

    
def recvIPpacket(interface):
    print("Interface: {}".format(interface))
    while(1):
        recievedPack = sniff(filter="ip",count=1, iface =conf.iface)
        #recievedPack.show()
        if(recievedPack[0].haslayer(Raw) and recievedPack[0][Ether].dst == get_if_hwaddr(conf.iface)):
            recievedPack.show()
            print("message Recieved:\t")
            print("Checksum {}".format(hex(recievedPack[0][IP].chksum)))
            recievedPack[0][Raw].show()
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
        sendIPpacket("h1-eth0","10.0.0.2", "10.0.0.1", "Hi this is some shit code")
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
        print("Usage:  python3 ./455_proj3.py Send <InterfaceName> <DestIP> <Router IP> <Message>\n\tpython3 ./455_proj3.py Send <InterfaceName>\n\tpython3 ./455_proj3.py debug <param>") 