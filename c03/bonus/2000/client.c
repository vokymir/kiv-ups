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
    else
    {
        printf("Invalid operand.\n");
        return 1;
    }

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

    printf("Sending: %s", request);

    if (send(sock, request, strlen(request), 0) < 0)
    {
        perror("send");
        close(sock);
        return 1;
    }

    int should_end = 0;
    do {
        // read response
        char buffer[5000];
        if ((recv(sock, buffer, sizeof(buffer)- 1, 0)) < 0)
        {
            perror("recv");
            close(sock);
            return 1;
        }

        buffer[4999] = '\0';

        // look for the line starting with "OK|" or "ERROR|"
        char *line = strtok(buffer, "\n");
        while (line) {
            if (strncmp(line, "OK|", 3) == 0) {
                printf("Result: %s\n", line + 3); // print res after OK
                should_end = 1;
                break;
            }
            else if (strncmp(line, "ERROR|", 6) == 0) {
                printf("Somwthing went wrong. %s\n", line + 6); // print res after ERROR
                should_end = 1;
                break;
            }
            line = strtok(NULL, "\n");
        }
    }while (should_end == 0);

    close(sock);
    return 0;
}
