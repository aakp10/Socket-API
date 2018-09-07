#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

#define P_READ 0
#define P_WRITE 3
#define C_WRITE 1
#define C_READ 2

int main()
{
    int pipe_fd[4];
    pid_t child_pid;
    int value;
    
    pipe(pipe_fd);
    pipe(pipe_fd + 2);


    if((child_pid == fork()) == 0)
    {
        /* close parent related ends of the pipe */
        close(pipe_fd[P_READ]);
        close(pipe_fd[P_WRITE]);
        /* receive value from the parent */
        int count_read;
        if (read(pipe_fd[C_READ], &value, sizeof(value)) > 0) {
            printf("child (%d) : read %d \n", getpid(), value);
        }

        printf("Child: Enter a value to send to the parent\n");
        scanf("%d", &value);
        /* write this value to parent after processing */
        write(pipe_fd[C_WRITE], &value, sizeof(value));
        printf("child(%d): wrote %d\n", getpid(), value);
        
        close(pipe_fd[C_READ]);
        close(pipe_fd[C_WRITE]);
        
        exit(0);        
    }
    else {
        close(pipe_fd[C_WRITE]);
        close(pipe_fd[C_READ]);

        printf("Parent: Enter a value to write to the child\n");
        scanf("%d", &value);

        printf("parent(%d): wrote %d\n", getpid(), value);
        write(pipe_fd[P_WRITE], &value, sizeof(value));
        
        if(read(pipe_fd[P_READ], &value, sizeof(value)) > 0) {
            printf("parent(%d): read %d\n", getpid(), value);
        }
        close(pipe_fd[P_READ]);
        close(pipe_fd[P_WRITE]);
        
        wait(NULL);
    }
    return 0;
}