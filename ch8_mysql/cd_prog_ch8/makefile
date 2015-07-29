all:  app

app: app_mysql.c app_testdriver.c app_mysql.h
	gcc -g -o app -I/usr/include/mysql app_mysql.c app_testdriver.c -lmysqlclient -L/usr/lib/mysql


