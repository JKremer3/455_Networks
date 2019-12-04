#Reciever

import socket
from scapy.all import *

UDP_IP = "10.0.0.2"
UDP_PORT = 5005
outfile = open("output.txt", "a+")

sock = socket.socket(socket.AF_INET, # Internet
                     socket.SOCK_DGRAM) # UDP
sock.bind((UDP_IP, UDP_PORT))

while True:
    data, addr = sock.recvfrom(1024) # buffer size is 1024 bytes
    if len(data.decode("UTF-8")) > 0:
        print(data.decode("UTF-8"), end = '')
        outfile.write(data.decode("UTF-8"))
    
    # Packet.__class__(bytes(data))
    # data.Show()
    #pckt=Raw(data)
    #pckt.show()

