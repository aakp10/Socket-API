#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define MAXLEN 80

int get_TCP_socket(char *ip, uint16_t port)
{
    int connection_fd =  socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_sockaddr;
    server_sockaddr.sin_family = AF_INET;
    server_sockaddr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &server_sockaddr.sin_addr);

    connect(connection_fd, (struct sockaddr*)&server_sockaddr, sizeof(server_sockaddr));

    return connection_fd;
}

int write_to_server(int connection_fd)
{
    char r_buffer[MAXLEN];
    printf("enter your name");
    ssize_t bytes_read;
    while((fgets(r_buffer, MAXLEN, stdin)) != NULL)
    {
        write(connection_fd, r_buffer, strlen(r_buffer));
        char w_buffer[MAXLEN];

        if((bytes_read = read(connection_fd, w_buffer, MAXLEN)) == 0)
        {
            perror("server terminated");
            return -1;
        }
        w_buffer[bytes_read] = '\0';

        fprintf(stdout, "%s \n", w_buffer);
    }

    return 0;
}
int main(int argc, char **argv)
{
    if(argc != 2)
    {
        printf("enter port number");
        return 0;
    }
    uint16_t port = strtol(argv[1], NULL, 0);
    int connection_fd = get_TCP_socket("0.0.0.0", port);

    write_to_server(connection_fd);
}