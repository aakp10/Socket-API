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
    printf("PARENT: Decrementing the value in heap by 2\n");
    *value -= 2;
    printf("PARENT: Address of %d value %p \n", *value, value);
}

void
child_worker(int *value)
{
    printf("CHILD: Incrementing the value in heap\n");
    (*value)++;
    printf("CHILD: Address of %d value %p \n", *value, value);
}

int main(int argc, char *argv[])
{
    int *shared_value = new_int(); 
    int child_status;

    printf("Num:\n");
    scanf("%d",shared_value);
    
    pid_t pid = fork();
    if(!pid) {
        child_worker(shared_value);
    }
    else {
        wait(&child_status);
        parent_worker(shared_value);        
    }
    return 0;
}