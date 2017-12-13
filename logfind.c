/*
Written by Dimitris Kokkonis
https://github.com/kokkonisd

Inspired by an exercise in Zed A. Shaw's book
"Learn C The Hard Way"
*/

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <ctype.h>
#include "logfind.h"

// free the 2D filename array's memory
void free_log_files (char **logfiles)
{
    int i = 0;
    while (strcmp(logfiles[i], ENDSTR) != 0) {
        if (logfiles[i])
            free(logfiles[i]);
        i++;
    }

    if (logfiles)
        free(logfiles);
}

// replace the tilde in the pathnames by the HOME path
char *replace_tilde (char *string)
{
    if (string[0] != '~')
        return string;

    char *result;

    int length = strlen(string) - 1 + strlen(getenv("HOME"));
    result = malloc((length + 1) * sizeof(char));

    strcpy(result, getenv("HOME"));
    strcat(result, ++string);

    return result;
}

// get the log filenames from ~/.logfind
char **get_log_files ()
{
    FILE *logs;
    char filename[MAX_PATH_SIZE];
    int rc, count = 0;
    char **log_files = NULL;

    // open the file and check that it opened correctly
    logs = fopen(replace_tilde(LOGFILE_LIST), "r");
    check(logs != NULL, "Couldn't open %s to get the list of log files. \
        \nMake sure you have permission to read it.", replace_tilde(LOGFILE_LIST));

    // count filenames in ~/.logfind
    rc = fscanf(logs, "%s", filename);
    while (!feof(logs)) {
        check(rc == 1, "Couldn't read filenames.");
        count++;
        rc = fscanf(logs, "%s", filename);
    }

    // allocate memory for the log filenames
    log_files = malloc((count + 1) * sizeof(char *));
    check(log_files != NULL, "Memory error.");

    // re-read the file, this time storing the names in log_files
    rewind(logs);
    
    for (int i = 0; i < count; i++) {
        rc = fscanf(logs, "%s", filename);
        check(rc == 1, "Couldn't read filenames.");

        // remove trailing newlines
        if (filename[MAX_PATH_SIZE - 1] == '\n')
            filename[MAX_PATH_SIZE - 1] = '\0';

        log_files[i] = malloc(MAX_PATH_SIZE * sizeof(char));
        check(log_files[i] != NULL, "Memory error.");

        log_files[i] = replace_tilde(filename);
    }

    /** 
     * put the end string as the last element
     * so we know when to stop reading file names
     */
    log_files[count] = ENDSTR;

    return log_files;

error:
    // failsafe exit
    if (logs) fclose(logs);
    if (log_files) free_log_files(log_files);
    return NULL;
}

// get a word from a file
char *get_word (FILE *fp) 
{
    char word[MAX_WORD_SIZE];
    int ch, i = 0;

    // skip stuff that isn't a word
    while (((ch = fgetc(fp)) != EOF) && !(isalpha(ch) || isdigit(ch)));

    // return NULL if there isn't any words left
    if (ch == EOF)
        return NULL;
    
    // copy word
    do {
        word[i++] = tolower(ch);
    } while (((ch = fgetc(fp)) != EOF) && (isalpha(ch) || isdigit(ch)));

    word[i] = '\0';

    return strdup(word);
}

// check if a string is in a file
int is_in_file (char *string, char *filename)
{
    FILE *f;
    f = fopen(filename, "r");
    // check if we have permission to read
    if (errno == EACCES)
        return 0;

    check(f != NULL, "Couldn't open file '%s'.", filename);

    char *word;
    word = get_word(f);

    // look for the word in the file
    while (word != NULL) {
        if (strcmp(word, string) == 0)
            return 1;
        word = get_word(f);
    }

    fclose(f);

    return 0;

error:
    if (f) fclose(f);
    return -1;
}

// parse arguments with logical AND
int parse_and (int argc, char *argv[])
{
    char **log_files = get_log_files();
    check(log_files != NULL, "Failed to get log files.");

    int i = 0;
    // while we haven't reached the end of filenames
    while (strcmp(log_files[i], ENDSTR) != 0) {
        int found = 1;
        // check if one of the terms is not found in current file
        for (int j = 1; j < argc; j++) {
            if (!is_in_file(argv[j], log_files[i])) {
                // logical AND is false
                found = 0;
                break;
            }
        }

        // if the file passed the test, print its name
        if (found)
            printf("%s\n", log_files[i]);

        i++;
    }

    free_log_files(log_files);

    return 0;

error:
    if (log_files) free_log_files(log_files);
    return -1;
}

// parse arguments with logical OR
int parse_or (int argc, char *argv[])
{
    char **log_files = get_log_files();
    check(log_files != NULL, "Failed to get log files.");

    int i = 0;
    // while we haven't reached the end of filenames
    while (strcmp(log_files[i], ENDSTR) != 0) {
        int found = 0;
        // check if at least one of the terms is found in current file
        for (int j = 1; j < argc; j++) {
            if (is_in_file(argv[j], log_files[i])) {
                // logical OR is true
                found = 1;
                break;
            }
        }

        // if the file passed the test, print its name
        if (found)
            printf("%s\n", log_files[i]);

        i++;
    }

    free_log_files(log_files);

    return 0;

error:
    if (log_files) free_log_files(log_files);
    return -1;
}


int main (int argc, char *argv[])
{
    check(argc >= 2, "Usage: %s [-o] search-term-1 search-term-2 ...\
        \nRun %s --help for a complete list of available commands", argv[0], argv[0]);

    check(argc != 2 || strcmp(argv[1], "-o") != 0, "Error: parameter -o requires at least one argument.");

    int rc;
    if (strcmp(argv[1], "-o") == 0) {
        rc = parse_or(argc, argv);
        check(rc != -1, "Couldn't parse arguments.");
    } else if (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0) {
        printf("%s\n", VERSION);
    } else if (strcmp(argv[1], "--help") == 0) {
        printf("Usage: %s [command] <search-term-1> <search-term-2> ...\
            \nAvailable commands:\n\t[none] : search for terms with logical AND\
            \n\t-o : search for terms with logical OR\
            \n\t-v, --version : show logfind's current version\
            \n\t--help : show this help screen\
            \nFor more details, refer to https://github.com/kokkonisd/logfind\n", argv[0]);
    } else {
        check(argv[1][0] != '-', "Unknown command '%s'\
            \nRun %s --help for a complete list of available commands", argv[1], argv[0]);

        rc = parse_and(argc, argv);
        check(rc != -1, "Couldn't parse arguments.");
    }

    return 0;

error:
    return -1;
}