#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <netdb.h> //gethostbyname
#include <fcntl.h>
#include <sys/time.h>

#define MAXLEN 100
#define PORT 80
#define BLOCK_SIZE 100
#define timeout 5

char*
get_dns_lookup(char *domain)
{
    struct hostent *host;
    struct in_addr *ip_addr;
    if((host = gethostbyname(domain)) == NULL) {
        fprintf(stdout,"error doing dns lookup");
    }
    ip_addr = (struct in_addr*) host->h_addr;
    return inet_ntoa(*ip_addr);
}

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
    printf("connection successful \n");
    return connection_fd;
}

int write_to_server(int connection_fd, char *path, char *host_ip)
{
    char http_request[MAXLEN];
    ssize_t bytes_read;
    //fgets(r_buffer, MAXLEN, stdin);
    /**		memset(block ,0 , BLOCK_SIZE);
     * Write to the socket
     * fd: of socket in estd connection
     * buffer ptr: stores the data to be written
     * len of buffer: how many bytes to be written across the socket
     */
    sprintf(http_request, "GET %s/ HTTP/1.1\r\nHost: %s\r\nContent-Type: text/plain\r\n\r\n", path, host_ip);
    printf("http request\n%s", http_request);
    write(connection_fd, http_request, strlen(http_request));

    /**
     * read data on the socket
     * fd: of the socket which is active in the data transfer or connected
     * buffer ptr: to which the data read is stored
     * len: bytes to be read into the buffer
     */
    //if()
    FILE *index = fopen("index.html", "w+");
    struct timeval begin , curr;
    int size_recv ;
    int total_size= 0;
	char block[BLOCK_SIZE];
	double timediff;
	fcntl(connection_fd, F_SETFL, O_NONBLOCK);
	gettimeofday(&begin , NULL);
	while(4)
	{
		gettimeofday(&curr , NULL);
		timediff =1e-6 * (curr.tv_usec - begin.tv_usec) + (curr.tv_sec - begin.tv_sec);
        memset(block ,0 , BLOCK_SIZE);
		if( total_size > 0 && timediff > timeout )
		{
			break;
		}

		if((size_recv =  read(connection_fd , block , BLOCK_SIZE ) )< 0)
		{
			usleep(200000);
		}
		else
		{
            block[size_recv] = '\0';
			fputs(block, index);
			total_size += size_recv;
			gettimeofday(&begin , NULL);
		}
	}

    fclose(index);
    int version, code;
    char status[30];
    char line[50];
    index = fopen("index.html", "r");
   fscanf(index,"%[^\n]", line);
    sscanf(line, "HTTP/1.1 %d %s",  &code, status);
    printf("status code %d message %s\n", code, status);
    printf("file name is index.html");
    fclose(index);
    return 0;
}

void
get_domain(char *url_val, char *domain, char *path)
{
    char url[100];
    strcpy(url, url_val);
    printf("%s\n", url_val);
    if(strncmp(url, "https://", strlen("https://")) == 0) {
        printf("can't handle https\n");
        exit(0);
    }

    char *dissect;
    int is_domain_set = 0;
    strcpy(path,"");
    int path_init = 0;
    while ((dissect = strtok_r(url_val, "/", &url_val))) 
        if(strncmp(dissect, "http:", strlen("http:")) != 0)
        {
            if(!is_domain_set){
                is_domain_set = 1;
                strcpy(domain, dissect);
            }
            else {
                if(!path_init)
                {
                    strcat(path,"/");
                    strcat(path, dissect);
                    path_init = 1;
                }
                else {
                    strcat(path,"/");
                    strcat(path, dissect);
                }
            }
        }

}

int main(int argc, char **argv)
{
    char domain[100], path[100];
    char *url;
    char *ip_address;
    int  choice;
    while((choice = getopt(argc, argv, "u:")) != -1)
    {
        if(choice == 'u')
            url = optarg;
    }
    char url_cpy[100];
    strcpy(url_cpy, url);
    get_domain(url, domain, path);
    printf("domain: %s\n path: %s\n", domain, path);
    ip_address = get_dns_lookup(domain);
    printf("%s mapped to %s\n", domain, ip_address);
    int connection_fd = get_TCP_socket(ip_address, PORT);
    write_to_server(connection_fd, path, domain);
    return 0;
}
