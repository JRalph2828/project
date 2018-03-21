#include "types.h"
#include "stat.h"
#include "user.h"

int main(int argc, char *argv[])
{
    int rc = fork();
    
    if (rc < 0)
    {
        printf(1, "fork failed\n");
        exit();
    }
    else if ( rc == 0)
    {
        printf(1, "My pid is %d\n", (int)getpid());
        printf(1, "My ppid is %d\n", (int)getppid());
    }
    else
    {
        wait();
    }
    
    exit();
}

