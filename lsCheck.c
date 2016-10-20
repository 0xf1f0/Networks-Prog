/*
 * This program displays the names of all files in the current directory.
 */
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <stdio.h>


/*
*Assuming we would not have more than 128 files in the directory, excluding "." and ".." which are the *current and parent directories respectively
*/
#define BUFFER_SIZE 128

//Function prototypes
void listFiles();
char *getfileIndex(int index);

int main(void)
{
	int i;
	int arr[BUFFER_SIZE];
	char *list[BUFFER_SIZE];
	char filename[BUFFER_SIZE];

	printf("\nList Files ++++++++++++++++\n");
	listFiles();

	strcpy(filename, getfileIndex(76));
	printf("\\nGet file index ++++++++++++++++\n");
	printf("%s\n", filename);

    FILE *fp = fopen(filename, "rb");
    if(fp == NULL)
    {
        perror("File");
        return 2;
    }
	return 0;
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
