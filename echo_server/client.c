#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>

#define MAXLEN 80

int get_TCP_socket(char *ip, uint16_t port)
{
    /**
     * Get a socket-fd belonging to
     * protocol Family : AF_INET: for IPv4
     * semantics of communication: SOCK_STREAM (for TCP)
     * which protocol to be used within the family : 0(If the family has 1 protocol supported
     */
    int connection_fd =  socket(AF_INET, SOCK_STREAM, 0);

    /**
     * sockaddr_in is socket address structure
     * store the socket details like
     * port
     * family: AF_INET: IPv4 support
     */
    struct sockaddr_in server_sockaddr;
    server_sockaddr.sin_family = AF_INET;
    /**
     * use htons to convert from host to network byte order
     */
    server_sockaddr.sin_port = htons(port);
    /**
     * Presentation to network byte order converter
     * Used for IPv4 w/ parameters:
     * AF_INET: family of the IP (here IPv4)
     * ip: ip to be converted
     * ptr to the in_addr field where network byte order IP is stored
     */
    inet_pton(AF_INET, ip, &server_sockaddr.sin_addr);

    /**
     * Connect to the server .
     *
     * Parameters:
     * fd of the socket to be used to refer to the client server connection.
     * ptr to the socket addr struct of the server
     * sizeof() the socket addr struct to know how many bytes to be copied
     */
    connect(connection_fd, (struct sockaddr*)&server_sockaddr, sizeof(server_sockaddr));

    return connection_fd;
}

int write_to_server(int connection_fd)
{
    char r_buffer[MAXLEN];
    printf("enter your name");
    ssize_t bytes_read;
    fgets(r_buffer, MAXLEN, stdin);
    /**
     * Write to the socket
     * fd: of socket in estd connection
     * buffer ptr: stores the data to be written
     * len of buffer: how many bytes to be written across the socket
     */
    write(connection_fd, r_buffer, strlen(r_buffer));
    char w_buffer[MAXLEN];

    /**
     * read data on the socket
     * fd: of the socket which is active in the data transfer or connected
     * buffer ptr: to which the data read is stored
     * len: bytes to be read into the buffer
     */
    if((bytes_read = read(connection_fd, w_buffer, MAXLEN)) == 0)
    {
        perror("server terminated");
        return -1;
    }
    w_buffer[bytes_read] = '\0';

    fprintf(stdout, "%s \n", w_buffer);
    /*
    USe this for continuous echo to server from stdin
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
    }*/

    return 0;
}
int main(int argc, char **argv)
{
    uint16_t port = 12345;

    int connection_fd = get_TCP_socket("0.0.0.0", port);
    write_to_server(connection_fd);

    return 0;
}