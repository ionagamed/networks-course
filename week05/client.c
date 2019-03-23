#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <memory.h>
#include "common.h"

test_struct_t client_data;
result_struct_t result;

void setup_udp_communication(char* server_ip_address, int server_port) {
	int sockfd = 0;
	int sent_recv_bytes = 0;
    socklen_t addr_len = sizeof(struct sockaddr);

    struct sockaddr_in dest;

    dest.sin_family = AF_INET;
    dest.sin_port = htons(server_port);
    struct hostent *host = (struct hostent *)gethostbyname(server_ip_address);
    dest.sin_addr = *((struct in_addr *)host->h_addr);

    sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sockfd < 0) {
		perror("socket creation: ");
		exit(1);
	}

    while(1) {
	    printf("Enter student name : ?\n");
	    scanf("%s", client_data.name);
	    printf("Enter age : ?\n");
	    scanf("%u", &client_data.age);
	    printf("Enter group : ?\n");
	    scanf("%s", client_data.group);

	    sent_recv_bytes = sendto(
			sockfd, 
		    &client_data,
		    sizeof(test_struct_t), 
		    0, 
		    (struct sockaddr *)&dest, 
		    sizeof(struct sockaddr)
		);
	    
	    printf("No of bytes sent = %d\n", sent_recv_bytes);

	    sent_recv_bytes = recvfrom(
			sockfd, 
			(char *)&result, 
			sizeof(result), 
			0,
		    (struct sockaddr *)&dest, 
			&addr_len
		);

	    printf("No of bytes received = %d\n", sent_recv_bytes);	    
	    printf("Result received = %s\n", result.message);
    }
}

int main(int argc, char **argv) {
	if (argc < 3) { 
		printf("Usage:\n    %s <server ip address> <server port>\n", argv[0]);
		exit(2);
	}

    setup_udp_communication(argv[1], atoi(argv[2]));
    printf("application quits\n");
    return 0;
}
