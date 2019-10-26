import sys
import socket
from scapy.all import *

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
    arpRes, noRes = srp(packet, timeout = 2)

    arpRes.show()
    if(arpRes.res[0][1].sprintf("%Ether.dst%") == get_if_hwaddr(conf.iface)):
        print("ARP received")
        arpRes.res[0][1].show()
    else:
        print("Failed to Receive ARP")

if __name__ == "__main__":
    global debug
    debug = 0
    if(len(sys.argv) > 1 and sys.argv[1] == "debug"):
        debug = 1

    getSystemInformation()
    
    ethhdr = createEthernetHeader()
    arphdr = createARPheader("10.0.0.2")
    packet = ethhdr/arphdr
    ethhdr.show()
    arphdr.show()
    print("\nFull Packet:")
    packet.show()

    processARP(packet)





