My approach to the homework was to split user input into tokens; token1 and token 2
Send the tokens to the server and wait for an ackknowledgement.
Case: ls client
	Compare the command and argument and remove ".." and  "." from the file listing
	Sort the list alphabetically and renumber them
	Display the result on screen
	
Case: ls server
	Send the command from client to server 
	The server prints its local file listings and save in a file; "lsServer.txt"
	The server send this file to the client.
	The client reads the file and print to screen.
	
case: u filenumber
	The client converts the filenumber to a filename and send the tokens to server
	Server responds with ackknowledgement
	The client sends the command and filename 
	The server saves the file.
	
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
	LS CLIENT
	u filenumber
	d filenumber
	bye/exit
	
What does not work?
	Issues with opening lsServer.txt
	ls server doesn't print to screen