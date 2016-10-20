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
#include <fcntl.h>
#include <dirent.h>

#define BUFFER_SIZE 20480   //20K
#define MAX_LEN 256
#define DELIMITER " \t\r\n\v\f"
#define UPLOAD_FAIL "Server: Upload failed"
#define WRITE_FAIL "Server: Write failed"
#define SERVER_WAITING "Server: waiting for connections"

// Number of waiting client
const int NUM_WC = 5;

int main(int argc , char *argv[])
{

    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    int sockListen;
    int sockAccept;
    unsigned int addrLen;
    unsigned int SERVER_PORT;
    ssize_t byteRcv = 0;    //byte read from local directory
    ssize_t byteSent = 0;   //byte sent to the socket
    ssize_t byteRead = 0;    //byte Read from the socket
    ssize_t fileSz = 0;     //Actual file size in byte

    // Check if correct arguments were passed
	if(argc != 2 || !(isdigit(*argv[1])))
	{
		fprintf(stderr, "Usage: %s PORT# \n", argv[0]);
		return  -1;
	}

    // Setup listening socket
    sockListen = socket(PF_INET, SOCK_STREAM, 0);
    if (sockListen < 0)
    {
        printf("Error creating the listening socket\n");
        exit(1);
    }

    // Setup address structure
	SERVER_PORT = atoi(argv[1]);

    memset((char *) &server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(SERVER_PORT);

    //Bind listening socket to server address
    if (bind(sockListen, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0)
    {
        printf("Failed to bind listening socket to address \n");
        exit(0);
    }

    //Listen for client's request
    if (listen(sockListen, NUM_WC) < 0)
    {
        printf("Failed to listen\n");
        exit(0);
    }
    addrLen = sizeof(client_addr);

    while(1)
    {
        printf("%s\n", SERVER_WAITING);
        //Establish a connection from client
        sockAccept = accept(sockListen,(struct sockaddr *) &client_addr, &addrLen);
        if (sockAccept < 0)
        {
            printf("Failed to accept connection\n");
            exit(1);
        }
        printf("Connection accepted from client: %s\n", inet_ntoa(client_addr.sin_addr));

        //Receive the command from client
        if(sockAccept > 0)
        {
            int msg4rmclient;
            char msgBuffer[BUFFER_SIZE];
            char fileBuffer[BUFFER_SIZE];

            msg4rmclient = recv(sockAccept, msgBuffer, sizeof(msgBuffer) - 1, 0);
            if (msg4rmclient == -1)
            {
                fprintf(stderr, "Failed to receive client's request\n");
                exit(1);
            }
            else if(msg4rmclient == 0)
            {
                printf("Connection closed by client: %s\n",inet_ntoa(client_addr.sin_addr));
                break;
            }

            else
            {
                /*Request from client was received successfully*/
                msgBuffer[msg4rmclient] = '\0';
                printf("Server: Request received from client [%s]\n", msgBuffer);
                if ((send(sockAccept, msgBuffer, strlen(msgBuffer), 0))== -1)
                {
                    fprintf(stderr, "Failure sending acknowledgment\n");
                    close(sockAccept);
                    break;
                }
            }
            /*Send an acknowledgment back to the client*/
            /*Process client request and parse it into tokens */
            //get the command and filename from client
            printf("Server: Acknowledgment sent [%s]\nNumber of bytes sent: %d\n", msgBuffer, strlen(msgBuffer));
            if(msg4rmclient > 0)
            {
                char *cmd;          //command
                char *fileName;     //filename
                int inFile; //file descriptor
                //char request[MAX_LEN]; //[command filename]
                //Make a copy of the request in order to compare
                //strcpy(request, msgBuffer);

                cmd = strtok(msgBuffer, DELIMITER);
                fileName = strtok(NULL, DELIMITER);
                //printf("Original request from client: %s\n", request);
                printf("command: %s\nfilename: %s\n", cmd, fileName);
                if(strcmp(cmd, "u") == 0) //Client want to upload a file
                {
                    /* Attempt to save received file on local directory using mode 0644 /rw-r--r--*/
                    inFile = open(fileName, O_WRONLY|O_CREAT, 0664);
                    if(inFile < 0)
                    {
                        printf("Error creating file \"%s\"\n", fileName);
                    }
                    else
                    {
                        /*
                        ssize_t byteRcv = 0;    //byte read from local directory
                        ssize_t byteSent = 0    //byte sent to the socket
                        ssize_t byteRead = 0    //byte Read from the socket
                        ssize_t fileSz = 0;     //Actual file size in byte
                        */
                        while((byteRcv = recv(sockAccept, fileBuffer, BUFFER_SIZE, 0)) > 0)
                        {
                            fileSz += byteRcv;
                            if(write(inFile, fileBuffer, byteRcv) < 0)
                            {
                                printf("Error writing \"%s\"\n", fileName);
                            }
                        }
                        close(inFile); //close the file
                    }
                    printf("File \"%s\" saved, %d bytes received\n", fileName, fileSz);
                }
                else
                {
                    printf("Unknown command from client\n");
                    break;
                }
            }
        }//close (sockAccept > 0)
    }
    //End of Inner While(1)...
    close(sockListen);	//Close the connection
}


void listFiles()
{
	struct dirent **namelist;
	int i;
	int n;
	int count = 1;

	n = scandir(".", &namelist, 0, alphasort);
	if(n < 0)
		perror("Error listing files");
	else
	{
		for(i = 0; i < n; i++)
		{
			if (!strcmp(namelist[i]->d_name, ".") || !strcmp(namelist[i]->d_name, ".."))
				continue;
			printf("\r%d.\t%s\n", count, namelist[i]->d_name);
			count++;
			free(namelist[i]);
		}
	}
	free(namelist);
	printf("\nTotal number of files: %d\n", count - 1);
	printf("\nEnter \"d filenumber\" to download a file\n");
}

char *getfileIndex(int index)
{
	struct dirent **namelist;
	int i;
	int n;
	int count = 1;

	n = scandir(".", &namelist, 0, alphasort);
	if(n < 0)
		perror("Error listing files");
	else
	{
		for(i = 0; i < n; i++)
		{
			if (index + 1 != i);
				free(namelist[i]);
		}
		return namelist[index + 1]->d_name;
	}
	free(namelist);
}

