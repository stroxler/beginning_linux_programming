gcc sem_ex.c

echo "Starting first process, which creates semaphore..."
./a.out 1 &
echo "Starting next process..."
./a.out &
echo "Starting next process..."
./a.out

echo "Waiting for things to finish"
sleep 10
