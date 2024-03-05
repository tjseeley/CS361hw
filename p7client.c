/*
    CS 361 Project 7
    p7client.c

    Authors: Thomas Seeley, Ryan Maring
*/

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

#include "network.h"

int main(int argc, char *argv[]) {
    int server_udp_socket;          // UDP socket for sending requests and receiving results
    struct sockaddr_in server_addr; // socket struct for server address
    uint16_t server_port;           // server port to be read from the command line
    struct comp_request request;    // struct for request message to be sent to server
    struct comp_result result;      // struct for result messge to be received from server
    //socklen_t receiver_size = sizeof(server_udp_socket);
    long int temp;                  // variable used to convert op1 and op2 command line arguments
    int32_t op1;                    // variable to hold op1 command line argument after conversion
    int32_t op2;                    // variable to hold op2 command line argument after conversion
    char *endptr;                   // used by strtol() for error checking

    // We require 5 command line parameters as specified in the usage
    if (argc != 6) {
        printf("Usage: %s server_ip_address server_port operator op1 op2\n", argv[0]);
        exit(0);
    }

    // see if the server port command line argument is a valid 2-byte integer
    server_port = (uint16_t) (strtol(argv[2], NULL, 0) & 0xffff);
    if (server_port == 0) {
        printf("Client: invalid server port: %s\n", argv[2]);
        exit(1);
    }

    // Make sure we only accept valid operator symbols
    if (strcmp(argv[3], "+") && strcmp(argv[3], "*") && strcmp(argv[3], "/")) {
      printf("Client: operator must be one of '+', '*', or '/'\n");
      exit(1);
    }

    // Try to convert the op1 command line parameter
    errno = 0;
    temp = strtol(argv[4], &endptr, 0);
    // Check if conversion succeeded. If not, invalid characters were part of the parameter
    if (errno != 0 || *endptr != '\0') {
      printf("Client: operand 1 is not a valid integer\n");
      exit(1);
    }
    // Check if it is a signed 4-byte integer
    if (temp > INT_MAX || temp < INT_MIN) {
      printf("Client: operand 1 is out of range\n");
      exit(1);
    }
    op1 = (int32_t)temp;

    // Try to convert the op2 command line parameter
    errno = 0;
    temp = strtol(argv[5], &endptr, 0);
    // Check if conversion succeeded. If not, invalid characters were part of the parameter
    if (errno != 0 || *endptr != '\0') {
      printf("Client: operand 2 is not a valid integer\n");
      exit(1);
    }
    // Check if it is a signed 4-byte integer
    if (temp > INT_MAX || temp < INT_MIN) {
      printf("Client: operand 2 is out of range\n");
      exit(1);
    }
    op2 = (int32_t)temp;

    /* At this point, all command line arguments except the IP address have been verified
       and properly converted.*/
    server_udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
    int test = inet_pton(AF_INET, argv[1], &server_addr.sin_addr);
    if(test == 0)
    {
      printf("Client: operand 1 %s was not a valid ip adress\n", argv[1]);
      close(server_udp_socket);
      exit(1); 
    }
    else if(test < 0)
    {
      printf("Invalid adress family %s\n", strerror(errno));
      close(server_udp_socket);
      exit(1);
    }
    else
    {
      int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
      server_addr.sin_family =  AF_INET;
      server_addr.sin_port = server_port;
      request.magic = MAGIC_CHK;
      request.operation = strtol(argv[3], &endptr, 0);
      request.op1 = op1;
      request.op2 = op2;
      printf("%d\n", server_addr.sin_port);
      if (bind(server_udp_socket, (struct sockaddr *) &server_addr, sizeof(struct sockaddr_in)) < 0) {
        printf("Error binding to socket: %s\n", strerror(errno));
        exit(1);
    }
      printf("Failed at sendto?\n");
      if(sendto(server_udp_socket, (void *)&request, sizeof(struct comp_request), 0, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_in)) < 0){
        printf("Failed to send message to server %s\n", strerror(errno));
        close(sockfd);
        close(server_udp_socket);
        exit(1);
      }
      printf("Failed before recvfrom");
      if(recvfrom(sockfd, (void*)&result, sizeof(struct comp_result), 0, NULL, NULL) < 0)
      {
        printf("Failed to recieve message from server %s\n", strerror(errno));
        close(sockfd);
        close(server_udp_socket);
        exit(1);
      }
      printf("failed after recvfrom\n");
      if(result.magic != MAGIC_CHK)
      {
        printf("Client: invalid reply from server\n");
        close(sockfd);
        close(server_udp_socket);
        exit(1);
      }
      else if(result.success == RES_ERR)
      {
        printf("Client: an error occurred during the computation\n");
        close(sockfd);
        close(server_udp_socket);
        exit(1);
      }
      else
      {
        printf("Client: the result is %d\n", result.value);
      }
      close(sockfd);
      close(server_udp_socket);
      
    }
    
    return 0;

}

