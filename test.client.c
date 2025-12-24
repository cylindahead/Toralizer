/* test_client.c - Demo program for Toralize */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <host> <port>\n", argv[0]);
        printf("Example: %s example.com 80\n", argv[0]);
        return 1;
    }
    
    const char *host = argv[1];
    int port = atoi(argv[2]);
    
    /* Resolve hostname to IP (SOCKS4 requires IP addresses) */
    struct hostent *he = gethostbyname(host);
    if (!he) {
        fprintf(stderr, "Cannot resolve '%s'\n", host);
        return 1;
    }
    
    /* Create socket */
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        return 1;
    }
    
    /* Set up destination address */
    struct sockaddr_in dest;
    memset(&dest, 0, sizeof(dest));
    dest.sin_family = AF_INET;
    dest.sin_port = htons(port);
    memcpy(&dest.sin_addr, he->h_addr_list[0], he->h_length);
    
    printf("Connecting to %s:%d via proxy...\n", 
           inet_ntoa(dest.sin_addr), port);
    
    /* This connect() will be intercepted by toralize.so */
    if (connect(sock, (struct sockaddr *)&dest, sizeof(dest)) < 0) {
        perror("connect");
        close(sock);
        return 1;
    }
    
    printf("Connected successfully!\n");
    
    /* Send a simple HTTP request */
    char request[256];
    snprintf(request, sizeof(request),
             "HEAD / HTTP/1.0\r\n"
             "Host: %s\r\n"
             "User-Agent: Toralize-Test/1.0\r\n"
             "\r\n", host);
    
    if (write(sock, request, strlen(request)) < 0) {
        perror("write");
        close(sock);
        return 1;
    }
    
    /* Read response */
    char response[1024];
    int bytes = read(sock, response, sizeof(response) - 1);
    if (bytes > 0) {
        response[bytes] = '\0';
        printf("Server response:\n%s\n", response);
    }
    
    close(sock);
    return 0;
}
