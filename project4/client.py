#Sender
import socket
from scapy.all import *

def getLocalIP():
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.connect(("10.0.0.1", 80))

    localIP = sock.getsockname()[0]
    sock.close()

    return localIP

def sendWindow(window, sendIP, PORT, count):
    while count < len(window):
        data = window[count]
        sock.sendto(bytes(str(count) + data, encoding='UTF-8'), (sendIP, PORT))
        count+=1

def generateWindows(data, maxWindowSize):
    windowHolder = []
    dataBuf = []

    for line in data: #for all of the data
        #if the data buffer is the max size of a window
        if len(dataBuf) == maxWindowSize:
            windowHolder.append(dataBuf)
            dataBuf = []
        #if the data buffer is smaller than the max size, but the current line is last line in the data set
        elif len(dataBuf) < maxWindowSize and line == data[-1]:
            dataBuf.append(line)
            dataBuf.append("ffff") 
            windowHolder.append(dataBuf)
        #otherwise fill the buffer
        else:
            dataBuf.append(line)
    return windowHolder

def receiveAcknowledgment(recvIP, PORT, sock):
    recv = None
    sock.settimeout(6)

    #while nothing has been recieved -
    while (recv == None) or (recv == '\x00') or (recv == ''):
        try:
            recv, addr = sock.recvfrom(1024)
            recv = recv.decode("UTF-8")

            if recv == 'ACK':
                return 10
            elif recv[0] == 'N':
                return int(recv[2])

        except(socket.timeout):
            print("Failed to Receive; Socket closing")
            sock.close()
            exit(1)

if __name__ == "__main__":
    if(len(sys.argv) < 3):
        print("Invalid Input Parameters, run as:\npython3 client.py <DestIP> <Filename> ")
        exit(1)

    windowSize = 10
    IP = sys.argv[1]
    PORT = 5005
    filename= sys.argv[2]
    localIP = getLocalIP()

    file = open(filename, 'r+')
    lines = file.readlines()

    print("IP:", localIP)
    print("port:", PORT)
    #print("message:", MESSAGE)print("Window Complete")print("Window Complete")

    sock = socket.socket(socket.AF_INET, # Internet
                        socket.SOCK_DGRAM) # UDP
    sock.bind((localIP, PORT))
    windowKeeper = generateWindows(lines, windowSize)

    #for each window
    for window in windowKeeper:
        ackNum = 0
        #while there are less acknowledged packets than the max window
        while ackNum < windowSize:
            sendWindow(window, IP, PORT, ackNum)
            ackNum = receiveAcknowledgment(IP, PORT, sock)
    
    #Once the data has completed being sent, send the terminate message
    sock.sendto(bytes("ffff", encoding='UTF-8'), (localIP, PORT))

    #else:
        #print("Usage:\t python3 client.py [IP] tux.txt")
