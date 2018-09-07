#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int *
new_int()
{
    return (int*) malloc(sizeof(int));
}

void
parent_worker(int *value)
{
    printf("PARENT(%d): Decrementing the value in heap by 2\n", getpid());
    *value -= 2;
    printf("PARENT(%d): Address : %p value: %d \n", getpid(), value, *value);
}

void
child_worker(int *value)
{
    printf("CHILD(%d): Incrementing the value in heap\n", getpid());
    (*value)++;
    printf("CHILD(%d): Address : %p value: %d \n", getpid(), value, *value);
}

int main(int argc, char *argv[])
{
    int *shared_value = new_int(); 
    int child_status;

    printf("Num:\n");
    scanf("%d",shared_value);
    printf("Initial Address: %p value: %d \n", shared_value, *shared_value);

    pid_t pid = fork();
    if(!pid) {
        child_worker(shared_value);
        exit(0);
    }
    else {
        parent_worker(shared_value); 
        wait(&child_status);       
    }
    return 0;
}