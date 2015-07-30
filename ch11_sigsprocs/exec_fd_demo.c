#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

/* As promised, here's a demo of using file descriptors with exec, and the
 * fact that exec preserves them. We want a program that will convert a
 * file's contents to uppercase and write to stdout.
 *
 * We wrote a simple program upper.c (compile it with `gcc upper.c -o upper`),
 * but it only deals with stdin / stdout.
 *
 * What to do? Well, here we write a wrapper: in this program we reopen
 * file descriptor 0 using `freopen`, using a file input from the command
 * line. Then we call exec to turn ourselves into upper. But now when upper
 * reads from stdin, it's actually reading from the file opened with fd 0,
 * which makes it do what we want!
 *
 * Obviously this is just a fun demo, but it shows how things work nicely.
 */

int main(int argc, char *argv[])
{
    char *filename;

    if(argc != 2) {
        fprintf(stderr, "usage: useupper file\n");
        exit(1);
    }

    // reopen stdin on the file given
    filename = argv[1];
    if(!freopen(filename, "r", stdin)) {
        fprintf(stderr, "could not redirect stdin to file %s\n", filename);
        exit(2);
    }

    // replace this program with upper
    execl("./upper", "upper", NULL);

    // don't forget to print a message and exit abnormally if exec fails.
    perror("could not exec ./upper");
    exit(3);
}
