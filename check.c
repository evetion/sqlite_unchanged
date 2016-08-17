// #include "sqlite/sqlite3.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>


/* display help */
int help() {
    printf("PoC for checking if SQLite database has been changed by comparing sqlite header with copy of header in database.\n");
    printf("For more information see the README.md file.\n\n");
    printf("Usage: sqlite_unchanged [-s | -c ] [file]\n");
    printf("\t-s or --set:   store header in db\n");
    printf("\t-c or --check: compare header with stored header\n");
    printf("\tfile: filename or path\n");
    return 1;
}

#define CHANGES 999
#define WAL 5

int main(int argc, char *argv[])
{
    bool check = true;
    bool exists = false;
    bool valid = false;

    FILE *sqldb;
    uint32_t changes;

    unsigned char header[100];
    unsigned char wal_read;
    unsigned char wal_write;
    unsigned char bytes[4];

    /* check number of arguments */
    if (argc != 3) {
        printf("number of arguments = %i\n", argc);
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
        perror ("Error opening sqlite file.");
        return 1;
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
            fseek(sqldb, 17, SEEK_SET);
            fwrite(&wal_read, 1, 1, sqldb);

            // Set WAL write
            wal_write = WAL;
            fseek(sqldb, 1, SEEK_CUR);
            fwrite(&wal_write, 1, 1, sqldb);

            // Write changes
            fseek(sqldb, 24, SEEK_SET);
            fwrite(&bytes, 4, 1, sqldb);

            // changes = bytes[3] | (bytes[2] << 8)  | (bytes[1] << 16) | (bytes[0] << 24);

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
        // fseek(sqldb, 24, SEEK_SET);
        changes = header[27] | (header[26] << 8)  | (header[25] << 16) | (header[24] << 24);
        
        printf("header change value is at %i\n", changes);
        printf("wal read value is at %u\n", wal_read);
        printf("wal write value is at %u\n", wal_write);

        if ((wal_read != WAL) || (wal_write != WAL) || (changes != CHANGES)) {
            perror ("File is not original.");
        } else {
            printf("File is original.");
        }
        

        fclose(sqldb);
    }
    return 0;
}
