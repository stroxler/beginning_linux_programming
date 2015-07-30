#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

/* In this program, we use the same signal api demod in basic_signal.c
 * in combination with fork() to set an alarm, where the child (which gets
 * 0 from fork(), remember) interrupts the parent with a SIGALARM and then
 * exits, and the parent catches the alarm in a handler which sets a global
 * var.
 *
 * The kill(pid, signal) function is used to send signals from one process
 * to another.
 *
 * The pause() function causes a program to stop running until it sees the
 * next signal. So when the parent hits that, it freezes until the child sends
 * the interrupt.
 *
 * This program, of course, would be better done with threads. But it's a
 * nice little demo of the apis.
 */

static int alarm_fired = 0;

void ding(int sig)
{
    alarm_fired = 1;
}

int main() {
    pid_t pid;

    printf("alarm application starting\n");

    pid = fork();
    switch(pid) {
    case -1:
      /* Failure */
      perror("fork failed");
      exit(1);
    case 0:
      /* child : waits 3 seconds, then sends alarm to parent */
        sleep(5);
        kill(getppid(), SIGALRM);
        exit(0);
    }

    // if we get here, we are in the parent
    printf("waiting for alarm to go off\n");
    (void) signal(SIGALRM, ding);

    pause();
    if (alarm_fired)
        printf("Ding!\n");

    printf("done\n");
    exit(0);
}
