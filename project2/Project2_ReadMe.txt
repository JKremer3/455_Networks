Executable compiled using gcc 

Instructions:
-------------
1) set up mininet network
	sudo mn
	
2) run "<interface> xterm &" on the two mininet nodes you will be running the code in.
	i.e. "h1 xterm &"

3) run "ipconfig -a" in the node that will be receiving to view the node's MAC address.

4)In the node that will be receiving run: ./455_Proj1_executable Recv <interfaceName>
	i.e. "./455_Proj1_executable Recv h2-eth0"

5)In the node that will be sending, run: ./455_Proj1_executable Send <interfaceName> <ReceivingMacAddress> <messageToSend>
	i.e. "./455_Proj1_executable Send 01:23:45:67:89:ab helloWorld"
	     "./455_Proj1_executable Send 01:23:45:67:89:ab 'This is a Message' "

6)Once the message has been sent, the program running on the sending node will end.
  Once the message has been received, the program on the receiving node will end.

NOTES:
------
a) make sure to run the program on the receiving node first. If you try to send a message to the receiving node when it is not listening, it will not receive the message
