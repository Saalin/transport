#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include "communication.h"

int main(int argc, char *argv[])
{
    if (argc != 5) {
        printf("Bad arguments length; should be 4: ipv4 port filename length");
        return EXIT_FAILURE;
    }

    FILE *file = fopen(argv[3], "w");
    if (file == NULL)
    {
        perror(argv[3]);
        return EXIT_FAILURE;
    }

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0) {
		perror("error");
		return EXIT_FAILURE;
	}

    download_file(sockfd, argv[1], atoi(argv[2]), atoi(argv[4]), file);

    close(sockfd);
    fclose(file);

    return EXIT_SUCCESS;
}