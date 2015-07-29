# You need to either #define _REENTRANT or use a -D in order to make standard
# C library calls be thread safe. It does a few things, including forcing most
# macros to become functions.

# on some machines you might need -I/user/include/nptl and -L/usr/lib/nptl

echo
echo "Running a simple thread example (just spin off and join, no syncing) ..."
gcc  -D_REENTRANT simple_example.c -lpthread
./a.out

echo
echo "Running a simple (underengineered) semaphore demo ..."
gcc  -D_REENTRANT semaphore_ex.c -lpthread
./a.out

echo
echo "Running a simple mutex demo ..."
gcc  -D_REENTRANT mutex_ex.c -lpthread
./a.out

echo
echo "Running a simple example using thread attrs ..."
gcc  -D_REENTRANT attrs_ex.c -lpthread
./a.out

echo
echo "Running a simple example using thread cancel ..."
gcc  -D_REENTRANT cancel_ex.c -lpthread
./a.out

echo
echo "Running a simple example using many threads ..."
echo "  -> it uses a dumb casting trick, so expect some warnings from gcc...."
gcc  -D_REENTRANT manythread_ex.c -lpthread
./a.out
