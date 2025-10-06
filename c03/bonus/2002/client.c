#include <arpa/inet.h>
#include <netinet/in.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        perror("socket");
        return 1;
    }

    struct sockaddr_in server;
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(2002);
    server.sin_addr.s_addr = inet_addr("147.228.67.67");

    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) != 0)
    {
        perror("connect");
        close(sock);
        return 1;
    }

    // get initial string
    int buf_len = 5000;
    char buffer[buf_len];

    if ((recv(sock, buffer, buf_len - 1, 0)) < 0)
    {
        perror("recv");
        close(sock);
        return 1;
    }

    buffer[buf_len - 1] = '\0';

    printf("%s",buffer);

    // reverse string
    size_t len = strlen(buffer) - 1;
    char response[len + 1 + 1]; // plus \n plus \0
    for (int i = 0; i < len; i++){
        response[i] = buffer[len - i - 1];
    }
    response[len] = '\n';
    response[len + 1] = '\0';

    printf("%s", response);

    // send answer back
    if (send(sock, response, len + 1, 0) < 0)
    {
        perror("send");
        close(sock);
        return 1;
    }

    memset(buffer, '\0', buf_len);

    // show if was ok or error
    if ((recv(sock, buffer, buf_len - 1, 0)) < 0)
    {
        perror("recv");
        close(sock);
        return 1;
    }

    printf("%s", buffer);

    close(sock);
    return 0;
}
