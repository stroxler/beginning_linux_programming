set -x

gcc fifo_reader_demo.c -o fifo_reader
gcc fifo_writer_demo.c -o fifo_writer

# make sure to start writer first, only because the reader
# doesn't mkfifo if it doesn't exist, whereas writer does
./fifo_writer &
sleep 1
./fifo_reader

rm fifo_reader
rm fifo_writer
