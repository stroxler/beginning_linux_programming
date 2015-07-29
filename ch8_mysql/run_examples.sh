echo
echo "Inserting sample data ...."
mysql -u trox -ptrox < children.sql

echo
echo "Connection example 1 ...."
gcc -I/usr/include/mysql connect_ex1.c -lmysqlclient
./a.out

echo
echo "Connection example 2 ...."
gcc -I/usr/include/mysql connect_ex2.c -lmysqlclient
./a.out

echo
echo "Prior to any changes, data is ...."
mysql -u trox -ptrox foo << EOF
select * from children;
EOF

echo
echo "Runing example mutation .... "
gcc -I/usr/include/mysql mutate.c -lmysqlclient
./a.out

echo
echo "After mutation, data is ...."
mysql -u trox -ptrox foo << EOF
select * from children;
EOF

echo
echo "Now, we demonstrate reading data in C ...."
gcc -I/usr/include/mysql read_data.c -lmysqlclient
./a.out
