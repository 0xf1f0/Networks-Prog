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
#include <stdbool.h>

#define BUFFER_SIZE 20480   //20K
#define MAX_LEN 256
#define DELIMITER " \t\r\n\v\f"
#define UPLOAD_FAIL "Server: Upload failed"
#define WRITE_FAIL "Server: Write failed"
#define SERVER_WAITING "Server: waiting for connections"

// Number of waiting client
const int NUM_WC = 5;

// Function prototype
char *getfileIndex(int index);
void lsServer();

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
            bool fileFound;
            int fileNum;

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
            //get the command and argument from client
            printf("Server: Acknowledgment sent [%s]\nNumber of bytes sent: %d\n", msgBuffer, strlen(msgBuffer));
            if(msg4rmclient > 0)
            {
                char *cmd;      //command
                char *arg;      //argument
                int inFile;     //file descriptor
                int outFile;    //file descriptor
                char *strptr;
                //char request[MAX_LEN]; //[command filename]
                //Make a copy of the request in order to compare
                //strcpy(request, msgBuffer);
                cmd = strtok(msgBuffer, DELIMITER);
                arg = strtok(NULL, DELIMITER);
                //printf("Original request from client: %s\n", request);

                if(strcmp(cmd, "u") == 0) //Client want to upload a file
                {
                    printf("command: %s\nfilename: %s\n", cmd, arg);
                    /* Attempt to save received file on local directory using mode 0644 /rw-r--r--*/
                    inFile = open(arg, O_WRONLY|O_CREAT, 0664);
                    if(inFile < 0)
                    {
                        printf("Error creating file \"%s\"\n", arg);
                    }
                    else
                    {
                        while((byteRcv = recv(sockAccept, fileBuffer, BUFFER_SIZE, 0)) > 0)
                        {
                            fileSz += byteRcv;
                            if(write(inFile, fileBuffer, byteRcv) < 0)
                            {
                                printf("Error writing \"%s\"\n", arg);
                            }
                        }
                        close(inFile); //close the file
                    }
                    printf("File \"%s\" saved, %d bytes received\n", arg, fileSz);
                }

                else if(strcmp(cmd, "d") == 0) //Client want to download a file
                {
                    char fileName[MAX_LEN];
                    fileNum = strtol (arg, &strptr, 10);     //strip the file number from token2 and store in base10
                    strcpy(fileName, getfileIndex(fileNum));
                    printf("command: %s\nFile name: %s\n", cmd, fileName);

                    if(fileName!= NULL)
                    {
                        //Check if the file exist before sending
                        //Send file to server
                        printf("Searching for file \"%s\" ...\n", fileName);
                        int outFile = open(fileName, O_RDONLY);   //file descriptor
                        if(outFile < 0)
                        {
                            fileFound = 1;
                            printf("File not found or Invalid file number\n");
                        }
                        else
                        {
                            /*
                            ssize_t byteRcv = 0;    //byte read from local directory
                            ssize_t byteSent = 0    //byte sent to the socket
                            ssize_t byteRead = 0    //byte Read from the socket
                            ssize_t fileSz = 0;     //Actual file size in byte
                            */
                            fileFound = 0;
                            printf("File \"%s\" found\n", fileName);
                            /* Read file in chunks of MAX_LEN =256 */
                            while((byteRead = read(outFile, fileBuffer, MAX_LEN)) > 0)
                            {
                                if(byteSent = send(sockAccept, fileBuffer, byteRead, 0) < 0)
                                {
                                    fprintf(stderr, "ERROR: Failed to send file %s\n", fileName);
                                }
                                fileSz += byteRead;
                            }
                            close(outFile);
                        }
                        printf("File \"%s\" uploaded successfully, %d bytes sent\n", fileName, fileSz);
                    }
                }

                else if(strcmp(cmd, "ls") == 0 && strcmp(arg, "server") == 0) //Client want to download a file
                {

                    //printf("The file is %s", ls_server); //verify
                    lsServer(); //create  "lsServer.txt" and Save the file listings to lsServer.txt
                    char fileName[MAX_LEN];
                    strcpy(fileName, "lsServer.txt");
                    outFile = open(fileName, O_RDONLY);   //Read only mode
                    if(outFile < 0)
                    {
                        fileFound = 1;
                        printf("File not found or Invalid file number\n");
                    }
                    else
                    {
                        fileFound = 0;
                        printf("File \"%s\" found\n", fileName);
                        /* Read file in chunks of MAX_LEN = 256 */
                        while((byteRead = read(outFile, fileBuffer, MAX_LEN)) > 0)
                        {
                            if(byteSent = send(sockAccept, fileBuffer, byteRead, 0) < 0)
                            {
                                fprintf(stderr, "ERROR: Failed to send file %s\n", fileName);
                            }
                            fileSz += byteRead;
                        }
                        close(outFile);
                    }
                    printf("File \"%s\" sent to client, %d bytes sent\n", fileName, fileSz);
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

/*Save the list of files in local directory to file*/
void lsServer()
{
    struct dirent **namelist;
	int i;
	int n;
	int count = 1;
	FILE *dest;
	n = scandir(".", &namelist, 0, alphasort);
	if(n < 0)
		perror("Error listing files");
	else
	{
	    /*Create a file called lsServer.txt and write the files and index to it*/
        dest = fopen("lsServer.txt", "wb+");
        if(dest == NULL)
            printf("Error creating file\n");
		for(i = 0; i < n; i++)
		{
			if (!strcmp(namelist[i]->d_name, ".") || !strcmp(namelist[i]->d_name, ".."))
				continue;
			//printf("\r%d.\t%s\n", count, namelist[i]->d_name);
            fprintf(dest, "\r%d.\t%s\n", count, namelist[i]->d_name);
			count++;
			free(namelist[i]);
		}

    }
	free(namelist);
}
