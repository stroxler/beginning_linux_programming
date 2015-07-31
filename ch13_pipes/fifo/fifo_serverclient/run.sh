set -x

make all

./server &

sleep 1

./client

rm server
rm client
