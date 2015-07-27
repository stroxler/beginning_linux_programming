# install using apt. You'll get prompted for 'root's password.
apt-get update
apt-get -y install mysql-server

# to log in as root, do the following:
## mysql -u root -p mysql
# which will lead to you being prompted for the password, then dropping
# into a mysql shell.
