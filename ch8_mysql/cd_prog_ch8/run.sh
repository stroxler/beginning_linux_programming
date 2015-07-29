mysql -u trox -ptrox < schema.sql
mysql -u trox -ptrox < sample_data.sql
make
./app
rm app
