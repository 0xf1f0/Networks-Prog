My approach to the homework was to split user input into tokens; token1 and token 2 by removing the new line character
Clients send the tokens to the server and wait for an ackknowledgement.
Server send an ackknowledgement based on the command and argument sent from clients.

Case: ls client
	Compare the command and argument and remove ".." and  "." from the file listing
	Sort the list alphabetically and renumber them
	Display the result on screen
	
Case: ls server
	Send the command from client to server 
	The server prints its local file listings and save in a file; "lsserver.txt"
	The server send this file to the client.
	The client reads the file and print to screen.
	
case: u filenumber
	The client converts the filenumber to a filename and send command and filename to server
	Server responds with acknowledgement
	Client sends the file.
	The server saves the file in its local directory.
	
case: d filenumber
	The client does "ls server"
	The client send the tokens to server
	Server converts the filenumber to a filename that matches the filenumber
	Server sends and ackknowledgement to client
	The server send the file to client
	The client receives the file and save it to local directory
	
case: bye/exit
	Clint closes the connection
	Server waits for other connections
	
	
What works?
	ls client
	multiple client connection to ftp server
	u filenumber
	d filenumber
	bye/exit
	
What does not work?
	Issues with opening lsserver.txt
	ls server sometimes won't print to screen
	Segmentation fault occurs when the file number does not exit
	
I also included testt2server.txt in the server directory and test2client.txt in the client directory for testing use cases.