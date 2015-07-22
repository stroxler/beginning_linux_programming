CFLAGS=-Wall
# on some distros just -lgdbm would work
LDFLAGS=-lgdbm_compat -lgdbm
# on some distros you might need INCLUDE=/usr/include/gdbm

# make by itself does whatever is at the top
all: clean application

clean:
	rm -f application *.o

application: app_ui.o cd_access.o
	gcc $(CFLAGS) -o application app_ui.o cd_access.o $(LDFLAGS)

app_ui.o: app_ui.c cd_data.h

cd_access.o: cd_access.c cd_data.h
