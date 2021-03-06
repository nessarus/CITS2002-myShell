#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

// testing pipe to cases

int main(void)
{
        int     fd[2]; 
        int     pid;
        char    string[] = "Hello, world!\n";
        char    readbuffer[80];

        pipe(fd);
        
        switch (pid = fork()) {
            case -1 :
                perror("fork");
                exit(1);

            case 0:
                /* closes up output side of pipe */
                close(fd[1]);

                /* read in a string from the pipe */
                read(fd[0], readbuffer, sizeof(readbuffer));
                printf("Received string: %s", readbuffer);
                exit(0);

            default:
                /* close up input side of pipe */
                close(fd[0]);
                /* send "string" through the output side of pipe */
                write(fd[1], string, (strlen(string)+1));
        }
        return(0);
}
