#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>

#define LISTEN_BACKLOG 9
#define MAXBUFLEN 80

void
process_request(int connection_fd)
{
    char r_buffer[MAXBUFLEN];
    char w_buffer[MAXBUFLEN+10];
    ssize_t bytes_read;
    while( (bytes_read = read(connection_fd, r_buffer, MAXBUFLEN)) > 0)
    {
        memcpy(w_buffer,"G'morning",9);
        strcat(w_buffer, r_buffer);
        write(connection_fd, w_buffer, strlen(w_buffer));
    }
}

int
start_server(uint16_t port)
{
    printf("port %d",port);
    /*Get a socket fd for the listening socket*/
    int listen_fd = socket(AF_INET /*domain of comm, for internet protocol ipv4*/, SOCK_STREAM, 0 /*which protocol to be supported within the family*/);
    /* Get a socket fd for the socket after connection is estd*/
    int connection_fd;
    socklen_t len = sizeof(struct sockaddr_in);

    /* Get a sock_addr struct*/
    struct sockaddr_in server_sockaddr, client_sockaddr;
    /* 0 fill the socket addr struct*/
    bzero(&server_sockaddr, sizeof(struct sockaddr_in));
    /* Initialize the values in sock_addr with server related fields*/
    server_sockaddr.sin_port = htons(port);
    server_sockaddr.sin_family = htonl(INADDR_ANY);
    /**
     * Use this for setting any other IP on your network
     */
    /*int retval_pton = inet_pton(AF_INET, "0.0.0.0", &server_sockaddr.sin_addr);
    if(retval_pton == 0)
    {
        perror("Invalid format of IP");
        return retval_pton;
    }
    else if(retval_pton == -1)
    {
        perror("Failed to convert from presentation to network byte order");
        return retval_pton;
    }*/

    server_sockaddr.sin_addr.s_addr = INADDR_ANY;

    /*bind the socket w/ socket addr struct server_sockaddr for listening i.e naming of the socket fd*/
    bind(listen_fd, (const struct sockaddr *) &server_sockaddr, sizeof(server_sockaddr));

    /* ASsign passive status to listen_fd for listening to active connection requests
     * with backlog queue length upto LISTEN_BACKLOG
     */
    listen(listen_fd, LISTEN_BACKLOG);

    while(1)
    {
        /*Accept the first connection in the pending connection
         *queue.
         *Creates a new socket and the fd to this new socket is stored 
         * in connetion_fd
         */
        connection_fd = accept(listen_fd, (struct sockaddr *) &client_sockaddr, &len);
        /*spawn the server*/
        if(fork() == 0)
        {
            /*
             *close the listening fd from the child
             *Only the connection fd is required to comm w/ the client
             */
            close(listen_fd);
            process_request(connection_fd);
            close(connection_fd);
            exit(0);
        }
        close(connection_fd);
    }
    return 1;
}

int main(int argc, char **argv)
{
    if(argc != 2)
    {
        printf("enter port number");
        return 0;
    }
    start_server(strtol(argv[1], NULL, 10));
    printf("Server Down");
    return 0;
}