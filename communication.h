#ifndef communication
#define communication 1

#include <stdio.h>
#define WINDOWSIZE 1000

void download_file(int sockfd, char *addr, int port, int length, FILE *file);

#endif