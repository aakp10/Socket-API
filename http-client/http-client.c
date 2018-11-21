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

#define MAXLEN 6000
#define PORT 80
#define CHUNK_SIZE 100
#define timeout 1

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
    /**
     * Write to the socket
     * fd: of socket in estd connection
     * buffer ptr: stores the data to be written
     * len of buffer: how many bytes to be written across the socket
     */
    sprintf(http_request, "GET /%s HTTP/1.1\r\nHost: %s\r\nContent-Type: text/plain\r\n\r\n", path, host_ip);
    printf("http request\n%s", http_request);
    write(connection_fd, http_request, strlen(http_request));
    char http_response[MAXLEN];

    /**
     * read data on the socket
     * fd: of the socket which is active in the data transfer or connected
     * buffer ptr: to which the data read is stored
     * len: bytes to be read into the buffer
     */
    /*
    USe this for continuous echo to server from stdin
    */
    char filename[40];
    //if()
    FILE *index = fopen("index", "w+");
   /* (bytes_read = read(connection_fd, http_response, MAXLEN));
    {
        http_response[bytes_read] = '\0';
        fputs(http_response, index);
    }
*/
    /*	int size_recv , total_size= 0;
	char block[CHUNK_SIZE];
		
		while((size_recv =  read(connection_fd , block , CHUNK_SIZE)) > 0)
		{fputs(block, index);
			memset(block ,0 , CHUNK_SIZE);	//clear the variable
		}
*/
int size_recv , total_size= 0;
	struct timeval begin , now;
	char block[CHUNK_SIZE];
	double timediff;
	
	//make socket non blocking
	fcntl(connection_fd, F_SETFL, O_NONBLOCK);
	
	//beginning time
	gettimeofday(&begin , NULL);
	
	while(1)
	{
		gettimeofday(&now , NULL);
		
		//time elapsed in seconds
		timediff = (now.tv_sec - begin.tv_sec) + 1e-6 * (now.tv_usec - begin.tv_usec);
		
		//if you got some data, then break after timeout
		if( total_size > 0 && timediff > timeout )
		{
			break;
		}
		
		//if you got no data at all, wait a little longer, twice the timeout
		else if( timediff > timeout*2)
		{
			break;
		}
		
		memset(block ,0 , CHUNK_SIZE);	//clear the variable
		if((size_recv =  read(connection_fd , block , CHUNK_SIZE ) )< 0)
		{
			//if nothing was received then we want to wait a little before trying again, 0.1 seconds
			usleep(100000);
		}
		else
		{
			total_size += size_recv;
			fputs(block, index);
			//reset beginning time
			gettimeofday(&begin , NULL);
		}
	}
    fclose(index);
    return 0;
}

void
get_domain(char *url_val, char *domain, char *path)
{
    char url[100];
    strcpy(url, url_val);
    printf("%s\n", url_val);
    /*if(strncmp(url, "https://", strlen("https://")) == 0) {
        sscanf(url, "https://%s", path);
    }
    else if(strncmp(url, "http://", strlen("http://")) == 0) {
        sscanf(url, "http://%s", path);
    }
    else {
        sscanf(url, "%s", path);
    }*/
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
