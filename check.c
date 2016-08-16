#include <stdio.h>
#include <string.h>
#include <stdbool.h>

/* minimum required number of parameters */
#define MIN_ARG 2
#define MAX_ARG 2

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

int main(int argc, char *argv[])
{
    bool check = true;
    int stream;
    int header[1];
    FILE *sqldb;

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
        /* header change value at 24..27 */
        fseek(sqldb, 24, SEEK_SET);
        fread(header, 4, 1, sqldb);
    }
    printf("header change value is at %i\n", *header);
    //       {
    //           n++;

    //           if(n >23 && n < 28)
    //           {
    //               printf("%x",c);    // printing the value of header whic in
    //               i++;
    //           }
    //       }
    fclose (sqldb);

    return 0;
}
