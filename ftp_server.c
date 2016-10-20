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
#include <dirent.h>

#define BUFFER_SIZE 20480   //20K
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
    printf("%s", SERVER_WAITING);
    addrLen = sizeof(client_addr);

    while(1)
    {
        //Establish a connection from client
        sockAccept = accept(sockListen,(struct sockaddr *) &client_addr, &addrLen);
        if (sockAccept < 0)
        {
            printf("Failed to accept connection\n");
            exit(1);
        }
        printf("Connection accepted from client: %s\n", inet_ntoa(client_addr.sin_addr));

        //Receive the command from client
        while(sockAccept > 0)
        {
            int msg4rmclient;
            char msgBuffer[BUFFER_SIZE];
            char fileBuffer[BUFFER_SIZE];
            int writeSz = 0;
            int clientSz = 0;

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
            /*Request from client was received successfully*/
            else
            {
                /*Process client request and parse it into tokens */
                //get the command and filename from client
                msgBuffer[msg4rmclient] = '\0';
                printf("Server: Command received from client [%s]\n", msgBuffer);

                char *cmd;          //command
                char *fileName;     //filename
                int fileBytes = 0;

                cmd = strtok(msgBuffer, DELIMITER);
                fileName = strtok(NULL, DELIMITER);

                printf("command: %s\nfilename: %s\n", cmd, fileName);

                while(1) //Command from client was received successfully
                {
                    if(strcmp(cmd, "u")) //Client want to upload a file
                    {
                        FILE *inFile = fopen(fileName, "ab");
                        if(inFile == NULL)
                        {
                            printf("Server: \"%s\" cannot be opened\n", fileName);
                            send(sockAccept, UPLOAD_FAIL, strlen(UPLOAD_FAIL), 0);
                            break;
                        }
                        else
                        {
                            while((fileBytes = recv(sockAccept, fileBuffer, BUFFER_SIZE, 0)) > 0)
                            {
                               writeSz = fwrite(fileBuffer, sizeof(char), clientSz, inFile);
                               if(writeSz < clientSz)
                               {
                                    printf("Server: \"%s\" cannot be opened\n", fileName);
                                    send(sockAccept, WRITE_FAIL, strlen(WRITE_FAIL), 0);
                                    break;
                               }
                               if(clientSz == 0  || clientSz != BUFFER_SIZE)
                                    break;
                            }
                        }
                    }
                    else
                    {
                        printf("Unknown command from client\n");
                        break;
                    }

                }
                if((send(sockAccept, msgBuffer, strlen(msgBuffer), 0)) == -1)
                {
                    fprintf(stderr, "Failure sending message\n");
                    close(sockAccept);
                    break;
                }
                printf("Server acknowledged request %s\nNumber of bytes sent: %d\n", msgBuffer, strlen(msgBuffer));
            }

        }
    }
    //End of Inner While...
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

