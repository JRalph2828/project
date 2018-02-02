#include "types.h"
#include "stat.h"
#include "user.h"

int main(int argc, char* argv[])
{
    int i;
        int rc = fork();

        if (rc < 0)
        {
             printf(1,"fork failed\n");
             exit();
        }
        else if ( rc == 0)
        {
            for(i=0;i<100;i++)
            {
                printf(1,"Child\n");
                yield();
            }
        }
        else
        {
            for(i=0;i<100;i++)
            {
                printf(1,"Parent\n");
                yield();
            }
        }
        wait();
        exit();


    return 0;
}