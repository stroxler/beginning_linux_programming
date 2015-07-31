echo "making fifo..."
mkfifo /tmp/fifo

echo "starting the cat ..."
cat < /tmp/fifo &

echo "listing jobs..."
jobs

echo "echoing into the fifo..."
echo "Data going into fifo... when echo exits, fifo is \
      closed and cat will also terminate" > /tmp/fifo
sleep 1

echo "listing jobs again..."
jobs
