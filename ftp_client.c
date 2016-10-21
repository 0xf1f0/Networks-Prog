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


#define BUFFER_SIZE 20480
#define MAX_LEN 256
#define DELIMITER " \t\r\n\v\f"

/*Pre-defined opcodes*/
#define COMMAND_OKAY 200
#define COMMAND_BAD 500
#define LS_SERVER 5
#define LS_CLIENT 4
#define FILE_UPLOAD 3
#define FILE_DOWNLOAD 2
#define CLIENT_EXIT 1
#define USAGE "Usage: command argument\n"

//Function prototypes
void direction(void);
void listFiles(void);
char *getfileIndex(int);

int main(int argc, char *argv[])
{
    char SERVER_IP[BUFFER_SIZE];
    int sock;
    int outfile;     //file descriptor of the file to upload
    int infile;     //file descriptor of the file to upload
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

    struct sockaddr_in server_addr;
    //bzero((char *) &sin_server, sizeof(sin_server));  //Set the first n bytes of server_addr to zeros
    memset((char *) &server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET; //Set to internet address type
	server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
	server_addr.sin_port = htons(SERVER_PORT);

	//Connect to the ftp server using socket and address structure
	int retcode;
	retcode = connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
	if(retcode < 0) //There is an error
	{
		printf("Failed to connect to the server: %s on port: %d\n", SERVER_IP, SERVER_PORT);
		exit(1);
	}
	/*No error were generated
	**Print the direction/how-to-use to the user
	**About time we do something now that we have successfully establish a connection with the server
	**Get ftp command + argument from keyboard
	*/
	direction();
    while(1)    //Keep command line interface in a continuous loop
    {
        char input[BUFFER_SIZE]; //user input from command line
        char copy[BUFFER_SIZE]; //copy of user input for comparison
        char *token1;   //command
        char *token2;   //argument
        char fileName[BUFFER_SIZE];
        char *strptr;
        char fileBuffer[BUFFER_SIZE];
        long int fileNum;
        bool fileFound;
        int inFile;     //file descriptor for incoming file
        int outFile;    //file descriptor outoing file
        char *msg2server;
        char serverAck[MAX_LEN];

        ssize_t byteRcv = 0;    //byte read from local directory
        ssize_t byteSent = 0;   //byte sent to the socket
        ssize_t byteRead = 0;    //byte Read from the socket
        ssize_t fileSz = 0;     //Actual file size in byte

        printf("ftp> ");    //Keep command line interface in a loop
        fgets(input, BUFFER_SIZE, stdin );

        /*Save a copy user input for later*/
        strcpy(copy, input);
        /*Process the user input and parse it into tokens */
        token1 = strtok(input, DELIMITER);
        token2 = strtok(NULL, DELIMITER);

        /* No input from user */
        /*Send the command and filename to server and wait for an acknowledgment*/
        /*Convert token2 to a number and convert to an integer */
        if(token1 == NULL)
        {
            continue;
        }
        /* **************************** */

        /* List Current files on client */
        else if(strcmp(token1, "ls") == 0 && strcmp(token2, "client") == 0)
        {
            listFiles(); //List the files in the current directory
        }
        /* **************************** */

        /* List Current files on server */
        else if(strcmp(token1, "ls") == 0 && strcmp(token2, "server") == 0)
        {
            printf("This is the msg2server: %s\n", copy);
            //msg2server: okay
            if ((send(sock, copy, strlen(copy), 0)) == -1)
            {
                fprintf(stderr, "File request failed\n");
                break;
                //Do something else or try again
            }
            else
            {
                printf("Waiting for acknowledgment from ftp server ...\n");
                retcode = recv(sock, serverAck, sizeof(serverAck), 0);
                if (retcode <= 0 )
                {
                    printf("Connection error, try again\n");
                    //Break from the while loop
                    break;
                }
                serverAck[retcode] = '\0';
                printf("Server Ack: '\%s\'\n", serverAck);
                //Concatenate the ack with ".txt"
                strcpy(fileName, serverAck);
                strcat(fileName, ".txt");
                //Download lsServer.txt */
                if(retcode > 0)
                {
                    /* Attempt to save received file on local directory using mode 0644 /rw-r--r--*/
                    inFile = open(serverAck, O_WRONLY|O_CREAT, 0664);
                    if(inFile < 0)
                    {
                        printf("Error creating file \"%s\"\n", fileName);
                    }
                    else
                    {
                        while((byteRcv = recv(sock, fileBuffer, BUFFER_SIZE, 0)) > 0)
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

                    FILE *fp;
                    int read;
                    fp = fopen("fileName","r");
                    while(1)
                    {
                        read = fgetc(fp);
                        if( feof(fp) )
                        {
                            break ;
                        }
                        printf("%c", read);
                   }
                    fclose(fp);//close file descriptor
                }
            }
        }
        /* **************************** */

        /* Upload a file to the server */
        else if((strcmp(token1, "u")== 0))
        {
            fileNum = strtol (token2, &strptr, 10);      //strip the file number from token2 and store in base10
            strcpy(fileName, getfileIndex(fileNum));
            //Append the filename to token1 separated by a space ' '
            //memset(msg2server, '\0', sizeof(msg2server));   //clear the memory location
            msg2server = malloc(strlen(token1) + strlen(" ") + strlen(fileName) + 1); //+1 for the zero-terminator
            strcpy(msg2server, token1);
            strcat(msg2server, " ");
            strcat(msg2server, fileName);
            printf("This is the msg2server: %s\n", msg2server);
            //msg2server: okay
            if ((send(sock, msg2server, strlen(msg2server), 0)) == -1)
            {
                fprintf(stderr, "File request failed\n");
                break;
                //Do something else or try again
            }
            else
            {
                printf("Waiting for acknowledgment from ftp server ...\n");
                retcode = recv(sock, serverAck, sizeof(serverAck), 0);
                if (retcode <= 0 )
                {
                    printf("Connection error, try again\n");
                    //Break from the while loop
                    break;
                }
                serverAck[retcode] = '\0';
                printf("Server Ack: '\%s\'\n", serverAck);
                /* Wait for message acknowledgment from server then send the files*/
                if(retcode > 0)
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
                            if(byteSent = send(sock, fileBuffer, byteRead, 0) < 0)
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
        }
        /* **************************** */

        /* Download a file from the server */
        else if((strcmp(token1, "d")== 0))
        {
            fileNum = strtol (token2, &strptr, 10);
            msg2server = malloc(strlen(token1) + strlen(" ") + strlen(token2) + 1); //+1 for the zero-terminator
            strcpy(msg2server, token1);
            strcat(msg2server, " ");
            strcat(msg2server, token2);
            printf("This is the msg2server: %s\n", msg2server);
            //msg2server: okay
            if ((send(sock, msg2server, strlen(msg2server), 0)) == -1)
            {
                fprintf(stderr, "File request failed\n");
                break;
                //Do something else or try again
            }
            else
            {
                printf("Waiting for acknowledgment from ftp server ...\n");
                retcode = recv(sock, serverAck, sizeof(serverAck), 0);
                if (retcode <= 0 )
                {
                    printf("Connection error, try again\n");
                    //Break from the while loop
                    break;
                }
                serverAck[retcode] = '\0';
                printf("Server Ack: '\%s\'\n", serverAck);
                /* Wait for message acknowledgment from server then send the files*/
                if(retcode > 0)
                {
                    /* Attempt to save received file on local directory using mode 0644 /rw-r--r--*/
                    inFile = open(token2, O_WRONLY|O_CREAT, 0664);
                    if(inFile < 0)
                    {
                        printf("Error creating file \"%s\"\n", token2);
                    }
                    else
                    {
                        while((byteRcv = recv(sock, fileBuffer, BUFFER_SIZE, 0)) > 0)
                        {
                            fileSz += byteRcv;
                            if(write(inFile, fileBuffer, byteRcv) < 0)
                            {
                                printf("Error writing \"%s\"\n", token2);
                            }
                        }
                    }
                    printf("File \"%s\" saved, %d bytes received\n", token2, fileSz);
                }
            }
        }
        /* **************************** */

        /* Exit the connection */
        else if((strcmp(token1, "bye") == 0) || strcmp(token1, "exit") == 0)
        {
            close(sock);
            break;
        }
        /* **************************** */

        /* Unknown commands */
        else
        {
            fprintf(stderr, USAGE);
        }

    }
    close(sock);	//Close the connection
    return 0;
}

/*Function to display menu and usage to a user*/
void direction()
{
    printf("\n");
    printf("\t*****************************************************\n");
    printf("\t*              FTP Server Implementation            *\n");
    printf("\t*                                                   *\n");
    printf("\t* AVAILABLE COMMANDS:                               *\n");
    printf("\t*                                                   *\n");
    printf("\t* (1)ls client: list files on the client            *\n");
    printf("\t* (2)ls server: list files on the server            *\n");
    printf("\t* (3)u filenumber: upload filenumber to server      *\n");
    printf("\t* (4)d filenumber: download filenumber from server  *\n");
    printf("\t* (5)bye/exit: Close the connection                 *\n");
    printf("\t*****************************************************\n");
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
			printf("\r\t%d. %s\n", count, namelist[i]->d_name);
			count++;
			free(namelist[i]);
		}
	}
	free(namelist);
	printf("\nTotal number of files: %d\n", count - 1);
	printf("\nEnter \"u filenumber\" to upload a file\n");
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
			{
				free(namelist[i]);
			}
		}
		return namelist[index + 1]->d_name;
	}
	free(namelist);
}



