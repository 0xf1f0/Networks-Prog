/* client-side program that uploads and download files from a server *	using ftp */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#define MAX_LEN 24		//Maximum length of each command line argument
#define BUFFER_SIZE 50	

//Function prototypes
int formatCommand(char [], char *[], int *);

int main(int argc, char *argv[])
{
    char SERVER_IP[MAX_LEN];    //xxx.xxx.xxx.xxx max of 15 characters
    int sock;
    unsigned int addrLen;
    unsigned int SERVER_PORT;

    /*It's safe to assume the worst case scenario of typing an invalid IP format or gibberish
    **Verifying the completeness of server IP at this stage may be an overkill
    **Usage: ./ftp_client 192.168.2.3 12000
    */

    // Check if correct number of arguments were passed
	if(argc != 3)
	{
		fprintf(stderr, "Usage: %s SERVER_IP PORT# \n", argv[0]);
		return  -1;
	}
	
    //create a socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock < 0)
    {
        printf("Failed to create a socket\n");
        exit(1);
    }

    /*Get SERVER_IP from argv[1] and SERVER_PORT from argv[2]
    **Initialize ftp server address structure
    */
	strcpy(SERVER_IP, argv[1]);
	SERVER_PORT = atoi(argv[2]);
	
    struct sockaddr_in sin_server;
    bzero((char *) &sin_server, sizeof(sin_server));  //Set the first n bytes of server_addr to zeros
    sin_server.sin_family = AF_INET; //Set to internet address type
	sin_server.sin_addr.s_addr = inet_addr(SERVER_IP);
	sin_server.sin_port = htons(SERVER_PORT);
	
	//Connect to the ftp server using socket and address structure
	int retcode;
	retcode = connect(sock, (struct sockaddr *)&sin_server, sizeof(sin_server));
	if(retcode < 0) //There is an error
	{
		printf("Failed to connect to the server: \'%s on port: %d'\n",SERVER_IP, SERVER_PORT);
		exit(1);		
	}
	
	/*No error were generated 
	**About time we do something
	**Get ftp command line from keyboard 
	*/
		printf("Connection established with \'%s: %d'\n",SERVER_IP, SERVER_PORT);	
		char command[MAX_LEN];	//User command 
		const char delimeter[2] = " ";	//Separates the command from the argument
		char *token;
		int i = 0;
	
		/*Usage: ftp> command arg*
		**Read user input from command line
		**Check the command and do something 
		**e.g ls[list directory content], u[upload file to server], d[download file from server], q[quit] 
		*/
		puts("ftp> ");
		fgets (command, MAX_LEN, stdin );
		token = strtok(command, delimeter);		//Get the first token
		
		/*Traverse through other tokens*/
		while(token != NULL)
		{
			printf( "%s ", token );
			token = strtok(NULL, delimeter);
		}	
		/*Examine each command and argument*/		
	close(sock);	//Close the connection	
}


//Function to get the command from shell, tokenize it and set the args parameter
int formatCommand(char cmdBuffer[], char *args[], int *flag)
{
    char command[MAX_LEN];	//User command 
	const char delimeter[2] = " ";	//Separates the command from the argument
	char *token;
	int i = 0;
	
	/*Usage: ftp> command arg*
	**Read user input from command line
	**Check the command and do something 
	**e.g ls[list directory content], u[upload file to server], d[download file from server], q[quit] 
	*/
	puts("ftp> ");
	token = strtok(command, delimeter);		//Get the first token
	
	/*Traverse through other tokens*/
	while(token != NULL)
	{
		printf( "%s ", token );
		token = strtok(NULL, delimeter);
	}	
	/*Examine each command and argument*/	
}

