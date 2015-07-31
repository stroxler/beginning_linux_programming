gcc consumer.c -o c
gcc producer.c -o p

./c &
./p

sleep 5

rm c
rm p

