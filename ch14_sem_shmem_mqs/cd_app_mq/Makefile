ll:	server client

CC=cc
CFLAGS= -Wall  # I got rid of -pedantic b/c it doesn't like C++ comments (//)

# For debugging un-comment the next line
# DFLAGS=-DDEBUG_TRACE=1 -g


# Where, and which version, of dbm are we using.
# This assumes gdbm is pre-installed in a standard place, but we are
# going to use the gdbm compatability routines, that make it emulate ndbm.
# We do this because ndbm is the 'most standard' of the dbm versions.
# Depending on your distribution, these may need changing.

DBM_INC_PATH=/usr/include/gdbm
DBM_LIB_PATH=/usr/lib
DBM_LIB_FILE=-lgdbm_compat -lgdbm
# On some distributions you may need to change the above line to include
# the compatibility library, as shown below.
# DBM_LIB_FILE=-lgdbm

.c.o:
	$(CC) $(CFLAGS) -I$(DBM_INC_PATH) $(DFLAGS) -c $<

app_ui.o: app_ui.c cd_data.h
cd_dbm.o: cd_dbm.c cd_data.h
client_f.o: clientif.c cd_data.h cliserv.h
mqueue_imp.o: mqueue_imp.c cd_data.h cliserv.h
server.o: server.c cd_data.h cliserv.h


client: app_ui.o clientif.o mqueue_imp.o
	$(CC) -o client  $(DFLAGS) app_ui.o clientif.o mqueue_imp.o

server:	server.o cd_dbm.o mqueue_imp.o
	$(CC) -o server -L$(DBM_LIB_PATH) $(DFLAGS) server.o cd_dbm.o mqueue_imp.o $(DBM_LIB_FILE)

clean:
	rm -f server client *.o *~
