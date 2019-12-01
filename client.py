import socket
from scapy.all import *

#if argc == 3:
    #UDP_IP = "192.168.1.86"
bufSize = 10
UDP_IP = "10.0.0.2"
UDP_PORT = 5005


file = open("tux.txt", 'r+')
lines = file.readlines()

print("UDP target IP:", UDP_IP)
print("UDP target port:", UDP_PORT)
#print("message:", MESSAGE)

sock = socket.socket(socket.AF_INET, # Internet
                    socket.SOCK_DGRAM) # UDP

lineCount = lines.__len__()
iterations = lineCount/bufSize
iterCount = 0
linesWritten = 0
i = 0

while iterCount < iterations:
    
    #fill the buffer with lines from the input file
    sendBuf = []
    while i < bufSize and linesWritten < lineCount:
        sendBuf.append(lines[linesWritten])
   #     print(sendBuf[i], end = '')
        linesWritten+=1
        i+=1
    i = 0
    
    #send the lines stored in the buffer
    while i < sendBuf.__len__():
        print(sendBuf[i], end = '') 
        sock.sendto(bytes(sendBuf[i], encoding='UTF-8'), (UDP_IP, UDP_PORT))
        i+=1
    i = 0

    iterCount+=1


#else:
    #print("Usage:\t python3 client.py [IP] tux.txt")
