#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* display help */
int help() {
    puts("PoC for checking if SQLite database has been changed by comparing sqlite header with copy of header in database.\n");
    puts("For more information see the README.md file.\n");
    puts("Usage: sqlite_unchanged [-s | -c ] [file]");
    puts("\t-s or --set:   store header in db");
    puts("\t-c or --check: compare header with stored header");
    puts("\tfile: filename or path");
    return(EXIT_FAILURE);
}

#define CHANGES 2147483647  // max uint32_t
#define WAL 1  // legacy mode

int main(int argc, char *argv[])
{
    bool check = true;

    FILE *sqldb;
    uint32_t changes;

    unsigned char header[100];
    unsigned char wal_read;
    unsigned char wal_write;
    unsigned char bytes[4];

    /* check number of arguments */
    if (argc != 3) {
        return help();
    }
       
    /* check mode */
    if (strcmp("-s", argv[1]) == 0 || strcmp("--set", argv[1]) == 0) {
        check = false;
    }
    else if (strcmp("-c", argv[1]) == 0 || strcmp("--check", argv[1]) == 0) {
        ; // do nothing
    }
    else {
        return help();

    }

    /* open sqlite file and read header */
    sqldb = fopen(argv[2],"rb+");
    if (!sqldb) {
        perror("Error opening sqlite file:");
        return(EXIT_FAILURE);
    }
    else
    {
        /* set mode */
        if (!check) {
            // Set changes and convert to
            // big endian byte array
            changes = CHANGES;

            bytes[0] = (changes >> 24) & 0xFF;
            bytes[1] = (changes >> 16) & 0xFF;
            bytes[2] = (changes >> 8) & 0xFF;
            bytes[3] = changes & 0xFF;

            // Set WAL read
            wal_read = WAL;
            fseek(sqldb, 18, SEEK_SET);
            fwrite(&wal_read, 1, 1, sqldb);

            // Set WAL write
            wal_write = WAL;
            fwrite(&wal_write, 1, 1, sqldb);

            // Write changes
            fseek(sqldb, 4, SEEK_CUR);
            fwrite(&bytes, 4, 1, sqldb);

            puts("Hardcoded values set, will now check for validity.");

        /* check mode */
        }
        fseek(sqldb, 0, SEEK_SET);

        /* read complete header */
        fread(header, 1, 100, sqldb);

        /*  file format write version.
        1 for legacy; 2 for WAL. */
        wal_read = header[18];
        /*  file format write version.
        1 for legacy; 2 for WAL. */
        wal_write = header[19];

        /* header change value at 24..27 */
        changes = header[27] | (header[26] << 8)  | (header[25] << 16) | (header[24] << 24);
        
        // printf("header change value is at %i\n", changes);
        // printf("wal read value is at %u\n", wal_read);
        // printf("wal write value is at %u\n", wal_write);

        if ((wal_read != WAL) || (wal_write != WAL) || (changes != CHANGES)) {
            puts("File is not original.");
        } else {
            puts("File is original.");
        }

        fclose(sqldb);
    }
    return(EXIT_SUCCESS);
}
