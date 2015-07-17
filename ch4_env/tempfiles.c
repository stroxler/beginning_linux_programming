/* page 150-155. The discussion of strftime / strptime on page
 * 154-156 might prove especially helpful */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int main() {
    // L_tmpnam is defined by your system libraries
    char tmpname[L_tmpnam];
    char *filename;
    FILE *tmpfp;

    // the beginning linux programming book demos using tmpnam and tmpfile.
    //   However, the utility of tmpfile is limited, because it only gives you
    //   a FILE*, so you can't recover the path.
    //   And tmpnam is considered unsafe; in OSX the compiler warns that it is
    //   deprecated, and in linux it explicitly says it is unsafe.
    //
    // on more modern unix flavors, you can use
    //    char * mktemp(char * template)
    // to get a valid name for a temp file, where the `template` arg is a
    // string with six trailing X characters, which will be replaced.
    //
    // You can get a low-level file descriptor instead:
    //     int * mkstemp(char * template),
    // but in general this can be an issue because you can't recover the path.
    //
    // The preferred approach seems to be mkdtemp, which can instantly make
    // a directory and also return the string.
    //    char * mkdtemp(char * template)
    //
    // This function *can't* be given a read-only c-string, which is why I
    // do all this copy nonsense: it modifies the template in-place, so if
    // you pass a read-only array, you get an error
    //   on osx it's a bus error
    //   on linux it's a seg fault
    //
    // Also, it doesn't automatically put the temp file in "/tmp"; if you
    // want that, you have to do it explicitly in your template.
    char *template_rom = "/tmp/my-temp-dir-XXXXXX";
    char *template_heap = (char *) malloc(100);
    if (!template_heap) {
        printf("Memory error");
        exit(1);
    }

    strcpy(template_heap, template_rom);

    filename = mkdtemp(template_heap);
    printf("Generated a temp dir name with `mkdtemp`: \"%s\"\n", filename);

    exit(0);
}
