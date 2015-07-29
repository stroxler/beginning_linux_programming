# install using apt. You'll get prompted for 'root's password.
apt-get update
apt-get -y install mysql-server
apt-get -y install libmysqlclient-dev 

# this drops the header file in
#    /usr/include/mysql/mysql.h
# and the client libraries in
#    /usr/lib/x86_64-linux-gnu/libmysqlclient*


# to log in as root, do the following:
## mysql -u root -p mysql
# which will lead to you being prompted for the password, then dropping
# into a mysql shell.
#    alternatively, you can use the -p with an argument to provide the password
# on the command line. This isn't secure if someone can see your history, but
# convenient if you aren't concerned. If you do this, don't put a space, e.g.
## mysql -u root -proot mysql

# if you want to run a script in mysql, there's no equialent of psql -f; just
# use the < shell operator to set a file as stdin

# the conf file that runs things winds up, on ubuntu with apt, living at
## /etc/mysql/my.cnf
