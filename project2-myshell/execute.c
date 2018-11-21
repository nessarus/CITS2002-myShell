#include "myshell.h"

/*
   CITS2002 Project 2 2017
   Name(s):     student-name1 (, student-name2)
   Student number(s):   student-number-1 (, student-number-2)
   Date:        date-of-submission
 */

// -------------------------------------------------------------------

//  THIS FUNCTION SHOULD TRAVERSE THE COMMAND-TREE and EXECUTE THE COMMANDS
//  THAT IT HOLDS, RETURNING THE APPROPRIATE EXIT-STATUS.
//  READ print_shellcmd0() IN globals.c TO SEE HOW TO TRAVERSE THE COMMAND-TREE


char* edit_cd_path(char *cd_path)
{
    cd_path = (char*) realloc(cd_path,sizeof(char)); //strlen(".:..") * sizeof(char));
    if(cd_path == NULL)
    {
        printf("Cannot allocate memory\n");
        exit( EXIT_FAILURE );
    }
    
    const char* current_path = ".";
    DIR *dirp = opendir(current_path);
    if(dirp == NULL)
    {
        perror( "Cannot open directory" );
        exit(EXIT_FAILURE);
    }
    
    struct dirent *dp;
    char fullpath[MAXPATHLEN];
    while((dp = readdir(dirp)) != NULL)
    {
        struct stat stat_buffer;
        struct stat *pointer = &stat_buffer;
        
        sprintf(fullpath, "%s/%s", ".", dp->d_name); // current_path, dp->d_name);
        
        if(stat(fullpath, pointer) != 0)
        {
            perror( "Cannot open stat" );
            //                        exit( EXIT_FAILURE );
        }
        else if( S_ISDIR( pointer->st_mode ))
        {
            printf("dp->d_name is %s\n",dp->d_name);
            /*
             static char buffer[strlen(cd_path)+strlen(dp->d_name)+strlen(":")];
             sprintf(buffer, "%s:%s", cd_path,dp->d_name);
             cd_path = buffer;
             */
            cd_path = realloc(cd_path,(strlen(cd_path)+strlen(dp->d_name)+2)*sizeof(char));
            if(cd_path == NULL)
            {
                printf("Cannot allocate cd_path memory\n");
                exit( EXIT_FAILURE );
            }
            sprintf(cd_path, "%s%s:", cd_path, dp->d_name);
        }
    }
    return cd_path;
}
    



void cd_not_path_execution(SHELLCMD *t)
{
        char previous_directory[MAXPATHLEN];
        getcwd(previous_directory, MAXPATHLEN);
        char *cd_path;
        cd_path = (char*) malloc(0); //strlen(".:..") * sizeof(char));
    
        //            char *argv_dir = t->argv[1];
        char *end_argv_dir;
        char *argv_dir = strtok_r(t->argv[1],"/", &end_argv_dir);
        while(argv_dir)
        {
            
            cd_path=edit_cd_path(cd_path);
            
            char *end_cd_path;
            char *temp=strtok_r(cd_path,":",&end_cd_path);
            bool dir_found = false;
            while(temp)
            {
                if(strcmp(temp,argv_dir)==0)
                {
                    printf("\n\n");
                    printf("temp is %s\n",temp);
                    
                    chdir(temp);
                    dir_found = true;
                    break;
                }
                temp = strtok_r(NULL,":",&end_cd_path);
            }
         
            if(!dir_found)
            {
                printf("%s: %s: No such file or directory\n", t->argv[0],t->argv[1]);
                chdir(previous_directory);
                break;
            }
            argv_dir = strtok_r(NULL,"/",&end_argv_dir);
        }
        free(cd_path);
        return ;
}

int time_execution(SHELLCMD *t)
{
    struct  timeval start;
    struct  timeval end;
    
    unsigned  long diff;
    gettimeofday(&start,NULL);
    for (int i=0;i<t->argc-1;i++)
    {
        t->argv[i]=strdup(t->argv[i+1]);
    }
    t->argv[t->argc-1]=NULL;
    t->argc--;
    int exitstatus_temp;
    exitstatus_temp=execute_shellcmd(t);
    gettimeofday(&end,NULL);
    diff = 1000000 * (end.tv_sec-start.tv_sec)+ end.tv_usec-start.tv_usec;
    printf("\n\nexecution time is %ld millionseconds\n",diff);
    return exitstatus_temp;
}

void find_path_execute(SHELLCMD *t)
{
    char *temp=strtok(PATH,":");
    while(temp)
    {
        char temp_path[strlen(temp)+strlen(t->argv[0])];
        strcpy(temp_path,temp);
        strcat(temp_path,"/");
        strcat(temp_path,t->argv[0]);
        //printf("temp_path is %s\n",temp_path);
        FILE *fp=fopen(temp_path,"r");
        if(fp!=NULL)
        {
            execv(temp_path,t->argv);
        }
        temp = strtok(NULL,":");

    }

    fprintf(stderr, "%s: command  not found\n",t->argv[0]);
    return ;
}

void exit_return_value(SHELLCMD *t)
{
    printf("11\n");
    if(strcmp(t->argv[0],"exit")!=0)
    {
        return;
    }
    printf("12\n");
    if(t->argc<=1)
    {
        exit(previous_exitstatus);
    }
    printf("13\n");
    if(strcmp(t->argv[1],"0")==0)
    {
        exit(EXIT_SUCCESS);
    }
    printf("14\n");
    if(strcmp(t->argv[1],"1")==0)
    {
        exit(EXIT_FAILURE);
    }
    printf("15\n");
    exit(previous_exitstatus);
}

bool sequential_execution(SHELLCMD *t, int *exitstatus)
{
    switch(t->type)
    {
        case CMD_SEMICOLON:
        {
            previous_exitstatus = execute_shellcmd(t->left);
            *exitstatus = execute_shellcmd(t->right);
            return true;
            break; 
        }
        case CMD_AND:
        {
            *exitstatus = execute_shellcmd(t->left) && execute_shellcmd(t->right);
            return true;
        }
        case CMD_OR:
        {
            *exitstatus = execute_shellcmd(t->left) || execute_shellcmd(t->right);
            return true;
        } 
        case CMD_COMMAND:
            break;
        case CMD_SUBSHELL:
        {
            pid_t subshell_pid;
            subshell_pid=fork();
            if(subshell_pid==0)
            {
                printf("subshell start\n");
                *exitstatus = execute_shellcmd(t->left);
                printf("subshell end\n");
            }
            else
            {
                wait(&subshell_pid);
                *exitstatus = subshell_pid;
                printf("parent_sub\n");
            }
        }
        case CMD_PIPE:
            break;
        case CMD_BACKGROUND:
            break;
    }

    return false;
}

int execute_shellcmd(SHELLCMD *t)
{
    int  exitstatus;
    //int return_value;

    printf("1\n");
    if(t == NULL) {         // hmmmm, that's a problem
        exitstatus  = EXIT_FAILURE;
        return exitstatus;
    }
    else {              // normal, exit commands
        exitstatus  = EXIT_SUCCESS;
    }
    printf("2\n");

    if( sequential_execution(t, &exitstatus) )
    {
        return exitstatus;
    }

    printf("3\n");
    exit_return_value(t);
    printf("13\n");
    
    if (strcmp(t->argv[0],"time")==0)
    {
        exitstatus=time_execution(t);
        return exitstatus;
    }
  
    printf("4\n");
    if(strcmp(t->argv[0],"cd")==0)
    {
        if(t->argc==1)
        {
            chdir(HOME);
        }
        else if(t->argv[1][0]=='/')
        {
            chdir(t->argv[1]);
        }
        else
        {
            cd_not_path_execution(t);
        }
        return exitstatus;
    }

    printf("5\n");
    pid_t fpid;
    fpid=fork();

    if(fpid==0)// child process
    {
        char * strtemp;
        strtemp=strchr(t->argv[0],'/');
        if(strtemp!=NULL)
        {
           execv(t->argv[0],t->argv);
        }
    printf("6\n");

        find_path_execute(t);
        exit(EXIT_FAILURE);
    }
    else // parent process
    {
        wait(&fpid);
    }


    printf("7\n");

    return exitstatus;
}

