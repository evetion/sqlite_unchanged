#include "sqlite/sqlite3.h"
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

#define CHANGES = 9999

int main(int argc, char *argv[])
{
    bool check = true;
    bool exists = false;
    bool valid = false;

    char *create_sql;
    char *drop_sql;
    char *insert_sql;
    char *pragma_sql;
    char *read_sql;

    FILE *sqldb;
    int  rc;
    sqlite3 *db;
    uint32_t changes;

    unsigned char header[100];
    unsigned char wal_read;
    unsigned char wal_write;

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
    sqldb = fopen(argv[2],"rb");
    if (!sqldb) {
        perror ("Error opening sqlite file.");
        return 1;
    }
    else
    {
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
        fclose (sqldb);
    }
    printf("header change value is at %i\n", changes);
    printf("wal read value is at %u\n", wal_read);
    printf("wal write value is at %u\n", wal_write);

    /* Open database */
    rc = sqlite3_open(argv[2], &db);
    if(rc){
        printf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return 1;
    } else {
        /* Check existing meta table
        so we know it exists and is correct
        - table
        - columns
        - row */
        exists = true;
        valid = true;
        
        /* Check database */
        if (check) {
            if (valid && exists) {
                if ((wal_read != 1) || (wal_write != 1)) {
                    perror("Database changed to WAL mode.");
                    return 1;
                } else {
                    // SQL READ
                    // COMPARE
                }
            } else {
                perror("Can't read table, database is not original.");
                return 1;
            }

        /* Set meta table */
        } else {
            // SET PRAGMA           
            if (!valid) {
                // DROP TABLE
                //
                exists = false;
            }
            if (!exists) {
                // CREATE TABLE
                exists = true;
            }
            // SET VALUES
            // SQL INSERT
            valid = true;
        }
    }
    sqlite3_close(db);
    return 0;
}
