#include "parser.h"
#include <sys/stat.h>

void printcmd(struct cmd *cmd)
{
    struct backcmd *bcmd = NULL;
    struct execcmd *ecmd = NULL;
    struct execcmd *ecmd2 = NULL;       // i added this
    struct listcmd *lcmd = NULL;
    struct pipecmd *pcmd = NULL;
    struct redircmd *rcmd = NULL;

    int i = 0;
    
    if(cmd == NULL)
    {
        PANIC("NULL addr!");
        return;
    }
    
    //  my vars
    int exitCode;

    switch(cmd->type){
        case EXEC:
            ecmd = (struct execcmd*)cmd;
            if(ecmd->argv[0] == 0)
            {
                goto printcmd_exit;
            }

            //  my code
            pid_t pidexec;
            pidexec = fork();
            if (pidexec == 0) {
                execvp(ecmd->argv[0], &ecmd->argv[0]);
            }
            else {
                //parent stuff
            }

            wait(&exitCode);
            if(WEXITSTATUS(exitCode) != 0) {
                MSG("Non-zero exit code (%d) detected\n", WEXITSTATUS(exitCode));
            }

            //

            /*
            MSG("COMMAND: %s", ecmd->argv[0]);
            for (i = 1; i < MAXARGS; i++)
            {            
                if (ecmd->argv[i] != NULL)
                {
                    MSG(", arg-%d: %s", i, ecmd->argv[i]);
                }
            }
            MSG("\n");
            */
            break;

        case REDIR:
            rcmd = (struct redircmd*)cmd;

            //printcmd(rcmd->cmd);

            if (0 == rcmd->fd_to_close)
            {
                //  my code

                pid_t pid;
                pid = fork();
                if (pid == 0) {
                    int fd = open(rcmd->file, O_RDONLY, S_IRUSR | S_IWUSR);
                    dup2(fd, 0);
                    printcmd(rcmd->cmd);
                    //execvp(((struct execcmd *)rcmd->cmd)->argv[0], &((struct execcmd *)rcmd->cmd)->argv[0]);
                    close(fd);
                }
                else {
                    wait(NULL);
                }
                MSG("... input of the above command will be redirected from file \"%s\". \n", rcmd->file);
            }
            else if (1 == rcmd->fd_to_close)
            {
                pid_t pid;
                pid = fork();
                if (pid == 0) {
                    int fd = open(rcmd->file, O_CREAT|O_RDWR|O_TRUNC, S_IRUSR|S_IWUSR);
                    dup2(fd, 1);
                    printcmd(rcmd->cmd);
                    //execvp(((struct execcmd *)rcmd->cmd)->argv[0], &((struct execcmd *)rcmd->cmd)->argv[0]);
                    close(fd);
                }
                else {
                    wait(NULL);
                }
                MSG("... output of the above command will be redirected to file \"%s\". \n", rcmd->file);
            }

            //  dup(temp, stdin) when overwriting input
            //  dup2(temp, stdout) when changing output
            //  basically revert og tableB

            //  call printcmd(rcmd->cmd);
            //  dup file descriptors a lot, temp var, idk
            else
            {
                PANIC("");
            }

            break;

        case LIST:
            lcmd = (struct listcmd*)cmd;

            //  my code

            printcmd((struct cmd*)lcmd->left);
            printcmd((struct cmd*)lcmd->right);
/*
            printcmd(lcmd->left);
            MSG("\n\n");
            printcmd(lcmd->right);
*/
            break;

        case PIPE:
            pcmd = (struct pipecmd*)cmd;

            //  my code 

            int pipefd[2];
            pipe(pipefd);

            pid_t pid = fork();
            pid_t pid2;
            if (pid == 0) {
                close(pipefd[0]);
                //dup2(pipefd[1], 1);
                dup2(pipefd[1], STDOUT_FILENO);
                close(pipefd[1]);
                execvp(((struct execcmd *)pcmd->left)->argv[0], &((struct execcmd *)pcmd->left)->argv[0]);
                //printcmd(pcmd->left);
                exit(127);
            }
            else {
                if (pcmd->right) {
                    pid2 = fork();
                }
                if (pid2 == 0) {
                    dup2(pipefd[0], 0);
                    close(pipefd[0]);
                    //execvp(((struct execcmd *)pcmd->right)->argv[0], &((struct execcmd *)pcmd->right)->argv[0]);
                    if (pcmd->right->type == 3) {
                        printcmd(pcmd->right);
                        exit(0);
                    }
                    else {
                        printcmd(pcmd->right);
                        exit(0);
                    }
                }
            }
            wait(NULL);
            if (pid != 0) {
                close(pipefd[1]);
            }
            if (pid2 != 0) {
                close(pipefd[0]);     
            }
            

            //

            //printcmd(pcmd->left);
            //MSG("... output of the above command will be redrecited to serve as the input of the following command ...\n");            
            //printcmd(pcmd->right);

            break;

        case BACK:
            bcmd = (struct backcmd*)cmd;

            pid_t pid5; 
            pid5 = fork();
            int proc;

            if (pid5 == 0) {
                setpgid(0, 0);
                printcmd(bcmd->cmd);
            }
            else {
                waitpid(pid5, &proc, WNOHANG);
            }

            //printcmd(bcmd->cmd);
            //MSG("... the above command will be executed in background. \n");    

            break;


        default:
            PANIC("");
    
    }
    
    printcmd_exit:

    return;
}


int main(void)
{
    static char buf[1024];
    int fd;

    setbuf(stdout, NULL);

    // Read and run input commands.
    while(getcmd(buf, sizeof(buf)) >= 0)
    {
        struct cmd * command;
        command = parsecmd(buf);
        printcmd(command); // TODO: run the parsed command instead of printing it
    }

    PANIC("getcmd error!\n");
    return 0;
}
