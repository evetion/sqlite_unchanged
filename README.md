# sqlite_unchanged
PoC for checking if SQLite database has been changed by hardcoding file change counter in header.

## Description
For a header description, see https://www.sqlite.org/fileformat2.html#section_1_2_6. 

The file change counter and wal modes are hardcoded:

offset size description
18      1   File format write version. Set to 1
19      1   File format read version. Set to 1
24      4   File change counter. Set to 2147483647

For each write transaction an sqlite database will have its file change counter incremented, thus providing a means to check if the database has been changed. Only in the non-default WAL mode this header might not be incremented, but this results in the file format version being set to 2, which we can also check. Changing back to legacy mode will increase the file change counter as well.

The only collision occurs if the file change counter overflows and returns to its hardcoded value after 2.147.483.647 write transactions.

## Usage
sqlite_unchanged [-s | -c ] [file]
        -s or --set:   store header in db
        -c or --check: compare header with stored header
        file: filename or path

./sqlite_unchanged -c test.sqlite
File is not original.

./sqlite_unchanged -s test.sqlite
Hardcoded values set, will now check for validity.
File is original.

./sqlite_unchanged -c test.sqlite
File is original.