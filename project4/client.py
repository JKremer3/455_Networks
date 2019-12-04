#Sender
import socket
from scapy.all import *


def generateWindows(data, maxWindowSize):
    windowHolder = []
    dataBuf = []

    for line in data: #for all of the data
        #if the data buffer is the max size of a window
        if len(dataBuf) == maxWindowSize:
            windowHolder.append(dataBuf)
            dataBuf = []
        #if the data buffer is smaller than the max size, but the current line is last line in the data set
        elif len(dataBuf) < maxWindowSize and line == data[len(data)-1]:
            dataBuf.append(line) 
            windowHolder.append(dataBuf)
        #otherwise fill the buffer
        else:
            dataBuf.append(line)
    
    return windowHolder

if __name__ == "__main__":
    if(len(sys.argv) < 3):
        print("Invalid Input Parameters, run as:\npython3 client.py <DestIP> <Filename> ")
        exit(1)

    windowSize = 10
    UDP_IP = sys.argv[1]
    UDP_PORT = 5005
    filename= sys.argv[2]

    file = open(filename, 'r+')
    lines = file.readlines()

    print("UDP target IP:", UDP_IP)
    print("UDP target port:", UDP_PORT)
    #print("message:", MESSAGE)

    sock = socket.socket(socket.AF_INET, # Internet
                        socket.SOCK_DGRAM) # UDP
    
    windowKeeper = generateWindows(lines, windowSize)

    for window in windowKeeper:
        for line in window:
            print(line, end='')
            sock.sendto(bytes(line, encoding='UTF-8'), (UDP_IP, UDP_PORT))

    #else:
        #print("Usage:\t python3 client.py [IP] tux.txt")
