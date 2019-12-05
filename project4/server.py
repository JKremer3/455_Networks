#Reciever

import socket
from scapy.all import *

def getLocalIP():
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.connect(("10.0.0.1", 80))

    localIP = sock.getsockname()[0]
    print(sock.getsockname()[0])

    sock.close()

    return localIP 

if __name__ == "__main__":

    if (len(sys.argv) < 2): #if the file arg is not passed, exit
        print("!Invalid Input Parameters!\nproper input:\tpython3 server.py <filename>")
        exit(1)

    IP = getLocalIP()
    PORT = 5005
    outfile = open(sys.argv[1], "a+")

    sock = socket.socket(socket.AF_INET, # Internet
                        socket.SOCK_DGRAM) # UDP
    sock.bind((IP, PORT))
    sock.settimeout(5)

    nextSeqNumber = 0
    print("Awaiting Data Transfer")
    while True:
        try:
            data, addr = sock.recvfrom(1024)        # buffer size is 1024 bytes
            data = data.decode("UTF-8")             #decode the byte sequence
            message = data[1:]                      #slice off the seq number to get message

            #if a packet is recieved containing "ffff", close the socket and exit
            if data == "ffff" or data[1:] == "ffff":
                print("Transmission Complete")
                sock.close()
                exit(0)

            #pull the sequence number from the packet and compare
            #to the next expected packet
            seqNum = int(data[0])
            if nextSeqNumber == seqNum: #if it is equal to the next expected packet, write
                outfile.write(message)
                nextSeqNumber +=1
            else:                       #otherwise send a NACK, for a dropped packet
                print("Packet dropped in transmission: ")
                print(nextSeqNumber)
                nack = "N:"+str(nextSeqNumber)
                sock.sendto(bytes(nack, encoding='UTF-8'), (addr[0], PORT))

            if nextSeqNumber == 10: #if the nextSeqNumber is 10, reset, a full window will has been recieved
                print("ACK Sent")
                nextSeqNumber = 0
                sock.sendto(bytes("ACK", encoding='UTF-8'), (addr[0], PORT))
    
        except(socket.timeout):
            print("Socket Timed Out, Exiting Program")
            sock.close()
            exit(1)

            #if len(data.decode("UTF-8")) > 0:
             #   print(data.decode("UTF-8"), end = '')
              #  outfile.write(data.decode("UTF-8"))
        
        # Packet.__class__(bytes(data))
        # data.Show()
        #pckt=Raw(data)
        #pckt.show()

