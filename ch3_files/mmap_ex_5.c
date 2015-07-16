#include <unistd.h>
#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdlib.h>

typedef struct {
    int integer;
    char string[24];
} Record;

#define NRECORDS 10

/* write records to data_file */
void create_records(char * data_file) {
    FILE *file;
    int i;
    Record record;
    file = fopen(data_file, "w+");
    for (i=0; i < NRECORDS; i++) {
        record.integer = i;
        sprintf(record.string, "record %d", i); // sprintf is *not* bounds safe
        fwrite(&record, sizeof(Record), 1, file);
    }
    fclose(file);
}

/* read records from data_file */
void print_records(char * data_file) {
    FILE *file;
    int i;
    Record record;
    file = fopen(data_file, "r");
    printf("-----------\n");
    for (i=0; i < NRECORDS; i++) {
        fread(&record, sizeof(Record), 1, file);
        printf("Record{%d, \"%s\"}\n", record.integer, record.string);
    }
    fclose(file);
}
    



int main(int argc, char *argv[]) {

    if (argc != 2) {
        printf("Usage: %s <file to use for binary file manip>\n", argv[0]);
    }

    // declarations
    Record record, *mapped;
    char *data_file = argv[1];
    int i, fdes;
    FILE *file;

    // write a bunch of records to the file
    create_records(data_file);

    print_records(data_file);

    // change record 4 by hand. This is what you would do without mmap
    file = fopen(data_file, "r+");
    fseek(file, 4 * sizeof(Record), SEEK_SET);
    fread(&record, sizeof(Record), 1, file);
    record.integer = 14;
    sprintf(record.string, "changed to %d", record.integer);
    fseek(file, 4 * sizeof(Record), SEEK_SET);
    fwrite(&record, sizeof(Record), 1, file);
    fclose(file);

    print_records(data_file);

    // now, do a similar update but use mmap. Note it uses a raw
    // file descriptor, not a FILE
    fdes = open(data_file, O_RDWR);
    mapped = (Record *)mmap(0, NRECORDS * sizeof(Record),
                            PROT_READ|PROT_WRITE, MAP_SHARED, fdes, 0);
    mapped[4].integer = 24;
    sprintf(mapped[4].string, "changed to %d", mapped[4].integer);
    msync((void *)mapped, NRECORDS * sizeof(Record), MS_ASYNC);
    munmap((void *)mapped, NRECORDS * sizeof(Record));
    close(fdes);

    print_records(data_file);

    // all done!
    return 0;
}
