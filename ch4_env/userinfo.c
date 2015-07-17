#include <sys/types.h>
#include <pwd.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main() {
    uid_t uid; gid_t gid;
    char* whoami;
    struct passwd *pw;   // it's not actually a password, it's a struct for an
                         // entry the /etc/passwd file.

    // get the calling user's uid / gid
    uid = getuid();
    gid = getgid();
    
    // getlogin() is unsafe and deprecated. Does not work on my box,
    // it returns "(null)" as a string(!)
    whoami = getlogin();
    printf("User's getlogin doesn't work: %s\n", whoami);
    printf("But getuid and getgid do... uid: %d  gid:%d\n", uid, gid);

    // you can get the passwd struct for a user by uid, or by name
    pw = getpwuid(uid);
    printf("is pw null? %p\n", pw);
    printf("\nGot passwd entry for uid %d\n", uid);
    printf("\tname=%s, uid=%d, gid=%d, home=%s, shell=%s\n",
           pw->pw_name, pw->pw_uid, pw->pw_gid, pw->pw_dir, pw->pw_shell);

    // technically I think I'm leaking memory here: should be freeing the
    // old pw stuff first, but this is just an api demo...
    whoami = pw->pw_name;
    pw = getpwnam(whoami);
    printf("\nGot passwd entry for username %s\n", whoami);
    printf("\tname=%s, uid=%d, gid=%d, home=%s, shell=%s\n",
           pw->pw_name, pw->pw_uid, pw->pw_gid, pw->pw_dir, pw->pw_shell);

    return 0;
}
