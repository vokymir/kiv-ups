#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    if (argc < 2){
        printf("Usage: %s <one-word-message>\n", argv[0]);
        return 1;
    }

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        perror("socket");
        return 1;
    }

    struct sockaddr_in server;
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(2001);
    server.sin_addr.s_addr = inet_addr("147.228.67.67");

    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) != 0)
    {
        perror("connect");
        close(sock);
        return 1;
    }

    char request[strlen(argv[1]) + 1];
    strcpy(request, argv[1]);
    request[strlen(argv[1])] = '\n';
    request[strlen(argv[1]) + 1] = '\0';

    printf("Sending: %s", request);

    if (send(sock, request, strlen(request), 0) < 0)
    {
        perror("send");
        close(sock);
        return 1;
    }

    // read response
    char buffer[5000];
    if ((recv(sock, buffer, sizeof(buffer)- 1, 0)) < 0)
    {
        perror("recv");
        close(sock);
        return 1;
    }

    buffer[4999] = '\0';

    printf("Received: %s", request);

    int len = strlen(request);
    if (strncmp(request, buffer, len) == 0) {
        printf("Server responded with the correct answer.\n");
    }
    else
    {

        printf("Not same strings of length: %i\nSent: %s\nReceived: %s\n", len, request, buffer);
    }

    close(sock);
    return 0;
}
