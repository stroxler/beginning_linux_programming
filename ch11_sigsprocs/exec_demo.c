#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

/* simple demo of exec: this process replaces itself a ps aux call. The
 * "Done." print never gets printed, and this process doesn't show up in
 * the ps aux list.
 *
 * The difference between this and a system call is that the system call
 * goes through a shell, and creates a *new* process, whereas using exec
 * you (a) avoid the shell, and (b) reuse this pid.
 *
 * The execlp is one of several exec functions; the p means it includes
 * the search path when looking for the file. The first argument is the
 * name of the binary to execute. The second argument is kind of confusing,
 * but because you aren't using a shell, you get to *choose* the value of
 * arg[0], the name that the new process will see for itself.
 *
 * The exec functions that don't have a v in their name take vararg inputs;
 * the ones that do take a char * const array[] as their second input. Some
 * of them also accept a final input which is an array of environment
 * variablse. The details are not worth discussing here, see the docs.
 *
 * Additionally, you can use environ to set environment variables prior to
 * an exec call.
 *
 * A call to exec(), importantly, preserves file descriptors. We'll see a
 * fun hack that uses this in  a later example.
 */

int main()
{
    printf("Running ps with execlp\n");
    execlp("ps", "ps", "ax", NULL);  // the authors use 0, not NULL, but gcc
                                     // gives a 'missing sentinel' warning. grr
    printf("Done.\n");
    exit(0);
}
