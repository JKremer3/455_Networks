import socket
from scapy.all import *

#if argc == 3:
    #UDP_IP = "192.168.1.86"
UDP_IP = "10.0.0.2"
UDP_PORT = 5005

file = open("tux.txt", 'r+')
lines = file.readlines()

print("UDP target IP:", UDP_IP)
print("UDP target port:", UDP_PORT)
#print("message:", MESSAGE)

sock = socket.socket(socket.AF_INET, # Internet
                    socket.SOCK_DGRAM) # UDP
for line in lines:
    sock.sendto(bytes(line, encoding='UTF-8'), (UDP_IP, UDP_PORT))
#else:
    #print("Usage:\t python3 client.py [IP] tux.txt")
