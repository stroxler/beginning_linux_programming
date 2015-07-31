gcc consumer_creator.c -o cc
gcc producer.c -o p

./cc &
./p

sleep 5

rm cc
rm p

