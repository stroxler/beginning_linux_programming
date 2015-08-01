gcc client.c -o client
gcc server.c -o server

echo "Starting server... "
./server &
sleep 1    # if you forget this, the clients might complain about the
           # socket not existing.

echo "ls -lF /tmp/server.sock..."
ls -lF /tmp/server.sock

echo "Starting client..."
./client &
./client &
./client &
./client &

sleep 1

echo "Killing jobs..."
jobs -l
jobs -l | awk '{print $3}' | xargs kill -9

rm client
rm server
