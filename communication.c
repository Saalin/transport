#include "communication.h"

#include <netinet/ip.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <strings.h>
#include <limits.h>

void prepare_buffers(int32_t *has_value, uint8_t *window)
{
    for(int i = 0; i < WINDOWSIZE * 100; i++)
    {
        window[i] = 0;
    }
    for(int i = 0; i < WINDOWSIZE; i++)
    {
        has_value[i] = 0;
    }
}

int check_if_resposne_ready(int sockfd)
{
    fd_set desriptors;
    FD_ZERO(&desriptors);
    FD_SET(sockfd, &desriptors);

    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 2000;

    return select(sockfd + 1, &desriptors, NULL, NULL, &tv);
}

void construct_message(char *msg, int start, int length)
{
    sprintf(msg, "GET %d %d\n", start, length);
}

void send_datagram(int sockfd, char *addr, int port, int start, int length)
{
    struct sockaddr_in address;
	bzero(&address, sizeof(address));
	address.sin_family= AF_INET;
	address.sin_port = htons(port);
	inet_pton(AF_INET, addr, &address.sin_addr);

    char msg[80];
    construct_message(msg, start, length);
    int send_retval = sendto(sockfd, msg, strlen(msg), 0, (struct sockaddr *)&address, sizeof(address));
    if(send_retval < 0)
        perror("sendto");
}

void save_buffer_to_file(int len, FILE *file, int32_t *has_value, uint8_t *window, int *current_offset)
{
    int count = 0;
    for(int i = 0; i < WINDOWSIZE && has_value[i] == 1; i++)
    {
        has_value[i] = 0;
        count++;
    }
    if(count > 0) {
        *current_offset = *current_offset + count * 1000;
        int bytes_to_write = count * 1000;
        if (*current_offset > len) // end of file
            bytes_to_write = (count - 1) * 1000 + len % 1000;
        fwrite(window, sizeof(uint8_t), bytes_to_write, file);
        memmove(window, window + count * 1000, (WINDOWSIZE * 1000 - count * 1000) * sizeof(uint8_t));
        memmove(has_value, has_value + count, (WINDOWSIZE - count) * sizeof(int));
        float done = ((float)*current_offset/len * 100) > 100 ? 100 : ((float)*current_offset/len * 100);
        printf("%.3f%% done\n", done);
    }
}

void receive_ready_datagrams(int sockfd, char *addr, int port, int32_t *has_value, uint8_t *window, int *current_offset)
{
    while(check_if_resposne_ready(sockfd))
    {
        struct sockaddr_in sender;
		socklen_t sender_len = sizeof(sender);
		uint8_t buffer[1024];

		ssize_t packet_len = recvfrom(sockfd, buffer, 1024, MSG_DONTWAIT, (struct sockaddr *)&sender, &sender_len);
	    if(packet_len < 0)
	    	perror("recvfrom");
        
        char sender_ip[20];
        inet_ntop(AF_INET, &(sender.sin_addr), sender_ip, sizeof(sender_ip));
        if(strcmp(sender_ip, addr) == 0 && port == ntohs(sender.sin_port))
        {
            char header[5];
            strncpy(header, (char *)buffer, 4);
            header[4] = '\0';
            if(strncmp(header, "DATA", 5) == 0) //check if DATA at the beginning
            {                
                char *rest;
                int start = (int)strtol((char *)(buffer + 5), &rest, 10);
                int length = (int)strtol(rest, NULL, 10);

                //header length
                char len_str[100];
                sprintf(len_str, "DATA %d %d\n", start, length);
                int header_length = strlen(len_str);

                uint8_t *data = buffer + header_length;
                if(start >= *current_offset && start < *current_offset + WINDOWSIZE * 1000)
                {
                    memcpy(window + ((start - *current_offset)), data, 1000);
                    has_value[(start - *current_offset) / 1000] = 1;
                }
            }
        }
    }
}

void download_file(int sockfd, char *addr, int port, int length, FILE *file)
{
    int32_t has_value[WINDOWSIZE]; //marker - if has_value[i] = 1 then window[i] = DATA CURRENT_OFFSET 1000
    uint8_t window[WINDOWSIZE * 1000];
    int current_offset = 0;

    prepare_buffers(has_value, window);
    while(current_offset < length)
    {
        for(int i = 0; i < WINDOWSIZE && current_offset + i * 1000 < length; i++)
        {
            send_datagram(sockfd, addr, port, current_offset + i * 1000, 1000);
        }

        receive_ready_datagrams(sockfd, addr, port, has_value, window, &current_offset);
        save_buffer_to_file(length, file, has_value, window, &current_offset);
    }
}