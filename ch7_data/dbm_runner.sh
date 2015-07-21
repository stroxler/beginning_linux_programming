# you probably will need to do something like this on ubuntu:
#   apt-get install -y  libgdbm-dev

set -x

# on some linuxes, you may need -I/usr/include/gdbm
# on ubuntu you *must* precede -lgdbm with -lgdbm_compat to get ndbm
gcc -Wall dbm_ex.c -lgdbm_compat -lgdbm
./a.out
