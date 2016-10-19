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

#define BUFFER_SIZE 1024

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
	int sock;
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
    printf("Server: waiting for connections...\n");

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

            //Receive the filename from client
        while(sockAccept > 0)
        {
            int byteReceived;
            char buffer[BUFFER_SIZE];

            byteReceived = recv(sockAccept, buffer, sizeof(buffer) - 1, 0);
            if (byteReceived == -1)
            {
                fprintf(stderr, "Failed to receive client request\n");
                exit(1);
            }
            else if(byteReceived == 0)
            {
                printf("Connection closed by client: %s\n",inet_ntoa(client_addr.sin_addr));
                break;
            }
            else
            {
                buffer[byteReceived] = '\0';
                printf("Server: Msg Received %s\n", buffer);

                if ((send(sockAccept, buffer, strlen(buffer), 0))== -1)
                {
                    fprintf(stderr, "Failure sending message\n");
                    close(sockAccept);
                    break;
                }
            }
            printf("Server: Msg being sent: %s\nNumber of bytes sent: %d\n", buffer, strlen(buffer));
        }
    }
    //End of Inner While...
    close(sock);	//Close the connection
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

