#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* this array of c-strings is guaranteed to be null-terminated
 * each entry is a string of the form "VAR=value" */
extern char **environ;

int main(int argc, char *argv[]) {

    char *var = "HOME";
    char *newvalue = "my_new_HOME_value";
    char **env;

    printf("The value of %s is %s\n", var, getenv(var));

    printf("\nThe whole env is.....\n");
    env = environ;
    while (*env) {
        printf("\t%s\n", *env);
        env++;
    }

    // note that in general strlen is not safe. In this case we've
    // hard-coded both strings so we can guarantee \0-termination.
    char *assignment = malloc(strlen(var) + strlen(newvalue) + 2);
    if (!assignment) {
        fprintf(stderr, "out of memory");
    }
    strcpy(assignment, var);
    strcat(assignment, "=");
    strcat(assignment, newvalue);
    printf("Changing the value of %s to %s\n", var, newvalue);
    int rcode = putenv(assignment);
    if (rcode != 0) {
        fprintf(stderr, "putenv failed");
        free(assignment);
        exit(1);
    }

    printf("\nNow the whole env is.....\n");
    env = environ;
    while (*env) {
        printf("\t%s\n", *env);
        env++;
    }

    return 0;
}
