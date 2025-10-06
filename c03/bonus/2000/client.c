#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    if (argc < 4)
    {
        printf("Usage: %s <num1> <+-x/> <num2>\n", argv[0]);
        return 1;
    }

    // get operand
    const char *operand;
    if (argv[2][0] == '+') operand = "plus";
    else if (argv[2][0] == '-') operand = "minus";
    else if (argv[2][0] == 'x') operand = "multiply";
    else if (argv[2][0] == '/') operand = "division";
    else operand = argv[2];

    // correct order for server
    char request[256];
    snprintf(request, sizeof(request), "%s|%s|%s\n", operand, argv[1], argv[3]);

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        perror("socket");
        return 1;
    }

    struct sockaddr_in server;
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(2000);
    server.sin_addr.s_addr = inet_addr("147.228.67.67");

    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) != 0)
    {
        perror("connect");
        close(sock);
        return 1;
    }

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

    // parse the line starting with "OK|"
    char *line = strtok(buffer, "\n");
    while (line) {
        if (strncmp(line, "OK|", 3) == 0) {
            printf("%s\n", line + 3); // print res after OK
            break;
        }
        line = strtok(NULL, "\n");
    }

    close(sock);
    return 0;
}
