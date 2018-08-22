#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <getopt.h>
#include <systemd/sd-daemon.h>

#define LISTEN_BACKLOG 9
#define MAXBUFLEN 80

void
process_request(int connection_fd)
{
    char r_buffer[MAXBUFLEN];
    ssize_t bytes_read;

    while( (bytes_read = read(connection_fd, r_buffer, MAXBUFLEN)) > 0)
    {
        r_buffer[bytes_read] = '\0';
        write(connection_fd, r_buffer, strlen(r_buffer));
    }
}

int
start_server(uint16_t port)
{
    int sd_count;
    int listen_fd, connection_fd;
    socklen_t len = sizeof(struct sockaddr_in);
    /* Get a sock_addr struct*/
    struct sockaddr_in server_sockaddr, client_sockaddr;
    if((sd_count = sd_listen_fds(0)) ==1)
    {
        listen_fd = SD_LISTEN_FDS_START + 0;
    }
    else
    {

        listen_fd = socket(AF_INET /*domain of comm, for internet protocol ipv4*/, SOCK_STREAM, 0 /*which protocol to be supported within the family*/);
        /* 0 fill the socket addr struct*/
        bzero(&server_sockaddr, sizeof(struct sockaddr_in));
        /* Initialize the values in sock_addr with server related fields*/
        server_sockaddr.sin_port = htons(port);
        server_sockaddr.sin_family = htonl(INADDR_ANY);
        server_sockaddr.sin_addr.s_addr = INADDR_ANY;

        bind(listen_fd, (const struct sockaddr *) &server_sockaddr, sizeof(server_sockaddr));
        listen(listen_fd, LISTEN_BACKLOG);
    }
    while(1)
    {
        /* Use libev for asynchronous responses*/
        connection_fd = accept(listen_fd, (struct sockaddr *) &client_sockaddr, &len);
        process_request(connection_fd);
        close(connection_fd);
    }
    return 1;
}

int main(int argc, char **argv)
{
    int port;
    start_server(1234);
    printf("Server Down");
    return 0;
}