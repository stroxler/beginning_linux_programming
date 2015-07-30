#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>


/* A simple fork demonstration.
 *
 * Exec takes your single process and replaces it with a new one. Fork is
 * the opposite in a sense: it preserves your original process and also creates
 * an almost exact copy of it. The only way to see the difference between the
 * two processes is to look at the pid variable, which will be 0 in the new
 * process and will be the pid of the newly started process in the parent.
 *  -> this convention seems backward in some ways, but it makes sense because
 *     the child can already use getpid or getppid to get it's and it's
 *     parent's pid's, so it doesn't need anything from fork(), whereas the
 *     parent process needs the pid from fork() in order to find it's child.
 *
 * Fork is similar to threading in some ways, but it's different in that (a)
 * there's more overhead, and (b) there's not a shared address space - the
 * forked process has its own copies of everything.
 *
 * the wait() function causes a process to wait until any of its child
 * processes die before resuming. It returns the pid of the child that died
 * (in this case there's only one child, so it's redundant, but in general
 * you might have many children), and if the stat_val int pointer is not
 * null, it writes status information which can be interpreted by <sys/wait.h>
 * functions such as WIFEXITED.
 *
 * There's also a waitpid() function you can use to wait for only one
 * particular child process to finish.
 */
int main()
{
    pid_t pid;
    char *message;
    int n;
    int exit_code;

    printf("fork program starting\n");
    pid = fork();
    switch(pid) 
    {
    case -1:
        exit(1);
    case 0:
        message = "This is the child";
        n = 5;
        exit_code = 37;
        break;
    default:
        message = "This is the parent";
        n = 3;
        exit_code = 0;
        break;
    }

    for(; n > 0; n--) {
        puts(message);
        sleep(1);
    }

    if(pid) {
        int stat_val;
        pid_t child_pid;

        child_pid = wait(&stat_val);

        printf("Child has finished: PID = %d\n", child_pid);
        if(WIFEXITED(stat_val))
            printf("Child exited with code %d\n", WEXITSTATUS(stat_val));
        else
            printf("Child terminated abnormally\n");
    }
    exit (exit_code);
}
