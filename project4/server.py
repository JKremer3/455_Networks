#Reciever

import socket
import sys

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
    dataReceived = set()
    print("Awaiting Data Transfer")
    while True:
        try:
            data, addr = sock.recvfrom(1024)        # buffer size is 1024 bytes
            data = data.decode("UTF-8")             #decode the byte sequence
            message = data[1:]                      #slice off the seq number to get message

            #pull the sequence number from the packet and compare
            #to the next expected packet
            seqNum = int(data[0])
            #
            #if a packet is recieved containing "ffff" and it is in sequence, close the socket and exit
            if (nextSeqNumber == seqNum) and data[1:] == "ffff":
                print("Transmission Complete")
                sock.sendto(bytes("close", encoding='UTF-8'), (addr[0], PORT))
                sock.close()
                exit(0)
            #if the packet is in sequence, and hasnt already been received, write to file
            elif (nextSeqNumber == seqNum) and (data not in dataReceived): #if it is equal to the next expected packet, write
                outfile.write(message)
                dataReceived.add(data)
                nextSeqNumber +=1
            else: #otherwise send a NACK, for a dropped packet
                requestedSeq = nextSeqNumber
                nack = "N:"+str(nextSeqNumber)
                sock.sendto(bytes(nack, encoding='UTF-8'), (addr[0], PORT))

            if nextSeqNumber == 10: #if the nextSeqNureRequestInProgress = 0mber is 10, reset, a full window will has been recieved
                print("ACK Sent")
                nextSeqNumber = 0
                sock.sendto(bytes("ACK", encoding='UTF-8'), (addr[0], PORT))
        except(socket.timeout):
            print("Socket Timed Out, Exiting Program")
            sock.close()
            exit(1)

