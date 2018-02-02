#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>



int main(int argc, char *argv[])
{
    char inputs[100];                               //Array that save input
    char *input_arr[100];                           //Pointer array that saves first tokens - for ";" 
    char *input_arr2[100];                          //Pointer array that saves second tokens - for " "
    int count = 0;                                  //Variable that counts first tokens
    int count2 = 0;                                 //Variable that counts second tokens
    int i,j;                                        //Variable for For loop
    int stream_err;                                 //Variable for remove stream errorcheck

    if(argc == 1)                                   //Interactive mode
    {
        while(1)
        {    
            printf("prompt> ");
            fgets(inputs, sizeof(inputs), stdin);   //Accept input
            inputs[ strlen(inputs)-1] = '\0';       //Remove new-line character
            count = 0;                              //Initialiazation variable

            if(strcmp(inputs, "quit") == 0)         //If quit comes than terminate
                break;

            if(feof(stdin) != 0)                    //If Ctrl+D comes than terminate
            {
                printf("\n");
                break;
            }

            input_arr[count] = strtok(inputs, ";"); //Save first tokens in *input_arr - removed ";"
            while(input_arr[count] != NULL)         
            {
                count++;
                input_arr[count] = strtok(NULL, ";");
            }
        
        
            for(i=0 ; i <= count ; i++)
            {
              int rc = fork();                      //Make child process
        
              if(rc<0)                              //Error check for fork failed
              {
                   fprintf(stderr,"fork failed\n");
                   exit(1);
              }
              else if (rc==0)                       //Child process go here
              {
                   count2 = 0;
                   input_arr2[count2] = strtok(input_arr[i], " ");  //Save second tokens in *input_arr2 - removed " "
                   while(input_arr2[count2++] != NULL)
                   {
                      input_arr2[count2] = strtok(NULL, " ");
                   }    

                   char *myargs[count2+1];          //+1 space for NULL
                   for(j=0 ; j<count2 ; j++)        
                       myargs[j] = input_arr2[j];
                   execvp(myargs[0], myargs);       //Execute command
                   printf("wrong command\n");       //Error check for wrong command
                   exit(1);                         //Terminate wrong process
              }
              else
                  continue;                         //Parent process go here
            }
            for(i=0 ; i <=count ; i++)              //Parent process waits untill all child processes wil be done
                wait(NULL);                     
        }
    }

        if(argc == 2)                               //Batchmode
        {
           FILE *fp = fopen(argv[1],"rt");          //Open batchfile

           if(fp == NULL)                           //Error check for make stream
           {
               printf("make stream error\n");
               return 1;
           }

            while(1)                                //The loop for read a file and excute commands as line-by-line
            {    

                if(fgets(inputs, sizeof(inputs), fp) == NULL)   //If file has no more datas than terminated 
                    break;

                inputs[ strlen(inputs)-1] = '\0';
                count = 0;

                if(strcmp(inputs, "quit") == 0)     //If file contains "quit" that will terminate at that line
                    break;

                printf("%s\n",inputs);              //Echo the commands

                input_arr[count] = strtok(inputs, ";");
   
                while(input_arr[count] != NULL)
                {
                    count++;
                    input_arr[count] = strtok(NULL, ";");
                }
        
        
                for(i=0 ; i <= count ; i++)
                {
                  int rc = fork();
        
                  if(rc<0)
                  {
                       fprintf(stderr,"fork failed\n");
                       exit(1);
                  }
                  else if (rc==0)
                  {
                       count2 = 0;
                       input_arr2[count2] = strtok(input_arr[i], " ");
                       while(input_arr2[count2++] != NULL)
                       {
                              input_arr2[count2] = strtok(NULL, " ");
                       }    
                       char *myargs[count2+1];
                       for(j=0 ; j<count2 ; j++)
                           myargs[j] = input_arr2[j];
                       execvp(myargs[0], myargs);
                       printf("wrong command\n");
                       exit(1);
                  }
                  else
                      continue;
                }
                for(i=0 ; i <= count ; i++)
                    wait(NULL);
            }
            stream_err = fclose(fp);                            //Close the file
            if(stream_err != 0)                                 //Error check for remove stream
            {
                printf("remove steam error\n");
                return 1;
            }
        }


    return 0;
}
