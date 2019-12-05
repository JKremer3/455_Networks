HOW TO USE:

1) have mininet running 
2) in one terminal run the command:		"python3 server.py <filename>"
	this will receive a transmission, and write it to the specified file
3) in a different terminal run the command:	"python3 client.py <DestIP> <Filename>"
	this will send the specified file to that destination

Warning: if packet drops occur when an acknowledgment is sent, data transmission will halt
Warning: if data arrives at the socket before the server is running, that data may be processed
