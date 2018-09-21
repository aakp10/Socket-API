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
#include <pthread.h>
#include <signal.h>
#include <wait.h>
#include <sys/select.h>
#include <errno.h>

#define LISTEN_BACKLOG 9
#define MAXBUFLEN 80

struct thread_args{
    int connection_fd;
    struct sockaddr_in *client_addr;
};

int processes_created = 0;
int active_processes = 0;

void
get_greetings(char *greetings)
{
    time_t curtime = time(NULL);
    struct tm *loc_time = localtime(&curtime);
    char morning[] = "Good morning ";
    char noon[] = "Good afternoon ";
    char evening[] = "Good evening ";
    char night[] = "Good night ";
    if(loc_time->tm_hour >= 4 && loc_time->tm_hour < 12)
        memcpy(greetings, morning, sizeof(morning));
    else if(loc_time->tm_hour >= 12 && loc_time->tm_hour <17)
        memcpy(greetings, noon, sizeof(noon));
    else if(loc_time->tm_hour >= 17 && loc_time->tm_hour < 20)
        memcpy(greetings, evening, sizeof(evening));
    else if((loc_time->tm_hour >= 20 && loc_time->tm_hour <= 24) ||
                ((loc_time->tm_hour >= 0 && loc_time->tm_hour < 4)))
        memcpy(greetings, night, sizeof(night));
}

void
async_wait(int signum)
{
    int child_exit_status;
    pid_t child_exited;
    if(signum == SIGCHLD)
    {
        active_processes--;
        child_exited = wait(&child_exit_status);
        if (WIFEXITED(child_exit_status)) {
            printf("Child exited with pid:%d with a status %d\n", child_exited, WEXITSTATUS(child_exit_status));
        }
    }
    printf("Active processes: %d Total processes: %d\n", active_processes, processes_created);
}

void
process_request(int connection_fd, struct sockaddr_in client_sockaddr )
{
    char r_buffer[MAXBUFLEN];
    char w_buffer[MAXBUFLEN+20];
    ssize_t bytes_read;
    char greetings[20];

    get_greetings(greetings);
    //printf("%s \n", greetings);
    /**
     * read data on the socket
     * fd: of the socket which is active in the data transfer or connected
     * buffer ptr: to which the data read is stored
     * len: bytes to be read into the buffer
     */
    while( (bytes_read = read(connection_fd, r_buffer, MAXBUFLEN)) > 0)
    {
        r_buffer[bytes_read] = '\0';
        memcpy(w_buffer,greetings, sizeof(greetings));
        /*Just a hack to ignore newline*/
        strncat(w_buffer, r_buffer, strlen(r_buffer)-1);
        strncat(w_buffer, "!", 1);
    /**
     * Write to the socket
     * fd: of socket in estd connection
     * buffer ptr: stores the data to be written
     * len of buffer: how many bytes to be written across the socket
     */
        write(connection_fd, w_buffer, strlen(w_buffer));
        //printf("%s \n", w_buffer);
    }
    //active_processes--;
    close(connection_fd);
    printf("IP:%s:%d\n",inet_ntoa(client_sockaddr.sin_addr), client_sockaddr.sin_port);
}

int
start_server(uint16_t port)
{
    /*Get a socket fd for the listening socket*/
    /**
     * Get a socket-fd belonging to
     * protocol Family : AF_INET: for IPv4
     * semantics of communication: SOCK_STREAM (for TCP)
     * which protocol to be used within the family : 0(If the family has 1 protocol supported
     */
    int listen_fd = socket(AF_INET /*domain of comm, for internet protocol ipv4*/, SOCK_STREAM, 0 /*which protocol to be supported within the family*/);
    /* Get a socket fd for the socket after connection is estd*/
    //int connection_fd;
    socklen_t len = sizeof(struct sockaddr_in);
    /**
     * sockaddr_in is socket address structure
     * store the socket details like
     * port
     * family: AF_INET: IPv4 support
     */
    /* Get a sock_addr struct*/
    struct sockaddr_in server_sockaddr, client_sockaddr;
    int connection_fd;
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

    //bind a signal to SIGUSR1
    signal(SIGCHLD, async_wait);

    fd_set readfs;
    struct timeval select_timeout;
    select_timeout.tv_sec = 10;
    FD_ZERO(&readfs);
    int maxfd = listen_fd;

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
        //FD_CLR(listen_fd, &readfs);
        //printf("select to be invoked");
        FD_SET(listen_fd, &readfs);
        int fd_set_count = select(maxfd+1, &readfs, NULL, NULL, &select_timeout);
        if(fd_set_count == -1 && errno != EINTR)
            perror("select failed");
        else if(fd_set_count)
        {
            printf("select triggered");
            /* This is kinda redundant as we just have one fd to be inspected and fd_set_count >0 will be due to this fd getting ready*/
            if(FD_ISSET(listen_fd, &readfs))
            {
                /*Accept now*/
                connection_fd = accept(listen_fd, (struct sockaddr *) &client_sockaddr, &len);
                /*spawn the server*/
                active_processes++;
                processes_created++;
                printf("Active processes: %d Total processes: %d\n", active_processes, processes_created);
                if (fork()==0) {
                    close(listen_fd);
                    process_request(connection_fd, client_sockaddr);
                    close(connection_fd);
                    exit(2);
                }
                close(connection_fd);
            }
        }
    }
    return 1;
}

int main(int argc, char **argv)
{
    int port;
    int choice;
    while((choice = getopt(argc, argv, "p:")) != -1)
    {
        if(choice == 'p')
            port = atoi(optarg);
    }

    start_server(port);
    printf("Server Down");
    return 0;
}