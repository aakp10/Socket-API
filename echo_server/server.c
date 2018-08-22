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

    /**
     * read data on the socket
     * fd: of the socket which is active in the data transfer or connected
     * buffer ptr: to which the data read is stored
     * len: bytes to be read into the buffer
     */
    while( (bytes_read = read(connection_fd, r_buffer, MAXBUFLEN)) > 0)
    {
        r_buffer[bytes_read] = '\0';
    /**
     * Write to the socket
     * fd: of socket in estd connection
     * buffer ptr: stores the data to be written
     * len of buffer: how many bytes to be written across the socket
     */
        write(connection_fd, r_buffer, strlen(r_buffer));
        //printf("%s ", w_buffer);
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
        /*Get a socket fd for the listening socket*/
        /**
         * Get a socket-fd belonging to
         * protocol Family : AF_INET: for IPv4
         * semantics of communication: SOCK_STREAM (for TCP)
         * which protocol to be used within the family : 0(If the family has 1 protocol supported
         */
        listen_fd = socket(AF_INET /*domain of comm, for internet protocol ipv4*/, SOCK_STREAM, 0 /*which protocol to be supported within the family*/);
        /* Get a socket fd for the socket after connection is estd*/
        /**
         * sockaddr_in is socket address structure
         * store the socket details like
         * port
         * family: AF_INET: IPv4 support
         */
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

        /**
         *bind the socket w/ socket addr struct server_sockaddr for listening i.e naming of the socket fd*
        *listen_fd: fd to refer to the server's listening socket
        *sockaddr_in : of the server's socket details
        *sizeof sockaddr_in: to know number of bytes to be checked in the sock addr struct
        */
        bind(listen_fd, (const struct sockaddr *) &server_sockaddr, sizeof(server_sockaddr));

        /* ASsign passive status to listen_fd for listening to active connection requests
        * with backlog queue length upto LISTEN_BACKLOG
        * lsiten_fd: socket which was bound to the server sockaddr_in
        */
        listen(listen_fd, LISTEN_BACKLOG);
    }
    while(1)
    {
        /*Accept the first connection in the pending connection
         *queue.
         *Creates a new socket and the fd to this new socket is stored 
         * in connetion_fd
         * listen_fd: fd of the socket on which the server accepts connection req.
         * client_sockaddr : ptr to store the peer sockaddr struct
         * len: how many bytes to be filled into the peer sockect addr struct
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
    int port;
    start_server(1234);
    printf("Server Down");
    return 0;
}