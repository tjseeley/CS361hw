 /*
    /*
    CS 361 Project 7
    p7server.c

    Authors: Thomas Seeley, Ryan Maring
*/
*/

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdint.h>
#include <arpa/inet.h>

#include "network.h"

int main(int argc, char *argv[]) {

    uint16_t req_listen_port;           // port number to listen for requests
    int requests_socket;		// Socket used to receive UDP requests
    struct sockaddr_in requests_address;// Our listening address
    struct sockaddr_in requestor;	// Address of the socket we received a request from
    socklen_t requestor_size = sizeof(requestor);		// Used by recvfrom
    struct comp_request request_msg;    // Buffer to receive the request message
    struct comp_result result_msg;      // Buffer to send the result message
    char name_buffer[INET_ADDRSTRLEN + 1];  // Buffer to hold IP address of requestor
    
    if (argc != 2) {
        printf("Usage: %s listen_port\n", argv[0]);
        exit(0);
    }

    /* Try to convert argv[1] to requests port number */
    req_listen_port = (uint16_t) (strtol(argv[1], NULL, 0) & 0xffff);
    if (req_listen_port == 0) {
        printf("Invalid requests port: %s\n", argv[1]);
        exit(1);
    }

    // Implement the server logic below

    requests_socket = socket(AF_INET, SOCK_DGRAM, 0);

    memset(&requests_address, 0, sizeof(requests_address));
    requests_address.sin_family = AF_INET;
    requests_address.sin_port = htons(req_listen_port);
    requests_address.sin_addr.s_addr = INADDR_ANY;
    
    if (bind(requests_socket, (struct sockaddr *) &requests_address, sizeof(struct sockaddr_in)) < 0) {
        printf("Error binding to socket: %s\n", strerror(errno));
        exit(1);
    }

    while(1)
    {
        recvfrom(requests_socket, (void*)&request_msg, sizeof(struct comp_request), 0, (struct sockaddr*)&requestor, &requestor_size);
        memcpy(name_buffer, &requestor.sin_addr.s_addr, INET_ADDRSTRLEN);
        name_buffer[INET_ADDRSTRLEN] = '\0';
        if(request_msg.magic != MAGIC_CHK)
        {
            printf("Server: received invalid magic value: 0x%08x from %s, port %d\n", request_msg.magic, name_buffer, requests_address.sin_port);
            close(requests_socket);
            exit(1);
        }
        else if(request_msg.operation != OP_ADD && request_msg.operation != OP_MULT && request_msg.operation != OP_DIV)
        {
            printf("Server: received invalid operation request: %d from %s, port %d\n",request_msg.operation, name_buffer, requests_address.sin_port);
            close(requests_socket);
            exit(1);
        }

        else
        {
            result_msg.magic = MAGIC_CHK;
            if(request_msg.operation == OP_ADD)
            {
                result_msg.value = request_msg.op1 + request_msg.op2;
                result_msg.success = RES_OK;
            }

            else if(request_msg.operation == OP_MULT)
            {
                result_msg.value = request_msg.op1 * request_msg.op2;
                result_msg.success = RES_OK;
            }

            else if(request_msg.operation == OP_DIV)
            {
                if(request_msg.op2 == 0)
                {
                    result_msg.value = 0;
                    result_msg.success = RES_ERR;
                }
                else
                {
                    result_msg.value = request_msg.op1 / request_msg.op2;
                    result_msg.success = RES_OK;
                }
                
            }
            int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
            if(sendto(sockfd, (void *)&result_msg, sizeof(struct comp_result), 0, (struct sockaddr *)&requestor, sizeof(struct sockaddr_in)) < 0)
            {
                printf("Failed to send message to client %s", strerror(errno));
                close(requests_socket);
                close(sockfd);
                exit(1);
            }
            close(sockfd);
        }
        
    }
}
