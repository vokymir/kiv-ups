#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>

int main(void)
{
    int client_socket;
    int return_value;
    char cbuf;
    int len_addr;
    struct sockaddr_in my_addr;

    // internet, stream = spojovane, TCP protokol
    client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (client_socket <= 0)
    {
        printf("Secket ERR\n");
        return -1;
    }

    memset(&my_addr, 0, sizeof(struct sockaddr_in));

    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(2000);
    my_addr.sin_addr.s_addr = inet_addr("147.228.53.17");

    return_value = connect(client_socket, (struct sockaddr *)&my_addr, sizeof(struct sockaddr_in));
    if (return_value == 0) printf("Connect - OK\n");
    else printf("Connect - ERR\n");

    return return_value;
}
