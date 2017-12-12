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
#include "dbg.h"

#define ENDSTR "END_OF_LOG_FILES"
// redundant, needs to be removed & fixed
#define LOGS "/var/log/"
#define LOGFILE_LIST "/.logfind"
#define MAX_WORD_SIZE 1024
#define MAX_PATH_SIZE (MAX_WORD_SIZE * 2)

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

char *replace_tilde (char *string)
{
    if (string[0] != '~')
        return string;

    char *result;

    int length = strlen(string) - 1 + strlen(getenv("HOME"));
    result = malloc(length * sizeof(char));

    strcpy(result, getenv("HOME"));
    strcat(result, ++string);

    return result;
}

char **get_log_files ()
{
    FILE *logs;
    char filename[MAX_PATH_SIZE];
    int rc, count = 0;
    char **log_files = NULL;

    int list_length = strlen(getenv("HOME")) + strlen(LOGFILE_LIST);
    char *list = malloc(list_length * sizeof(char));
    strcpy(list, getenv("HOME"));
    strcat(list, LOGFILE_LIST);

    logs = fopen(list, "r");
    check(logs != NULL, "Couldn't open %s to get the list of log files. \
        \nMake sure you have permission to read it.", list);

    free(list);

    rc = fscanf(logs, "%s", filename);
    while (!feof(logs)) {
        check(rc == 1, "Couldn't read filenames.");
        count++;
        rc = fscanf(logs, "%s", filename);
    }

    log_files = malloc((count + 1) * sizeof(char *));
    check(log_files != NULL, "Memory error.");

    rewind(logs);
    
    for (int i = 0; i < count; i++) {
        rc = fscanf(logs, "%s", filename);
        check(rc == 1, "Couldn't read filenames.");

        if (filename[MAX_PATH_SIZE - 1] == '\n')
            filename[MAX_PATH_SIZE - 1] = '\0';

        log_files[i] = malloc(MAX_PATH_SIZE * sizeof(char));
        check(log_files[i] != NULL, "Memory error.");

        log_files[i] = replace_tilde(filename);
    }

    log_files[count] = ENDSTR;

    return log_files;

error:
    if (logs) fclose(logs);
    if (log_files) free_log_files(log_files);
    return NULL;
}

char *get_word (FILE *fp) 
{
    char word[MAX_WORD_SIZE];
    int ch, i = 0;

    // skip stuff that isn't a word
    while (((ch = fgetc(fp)) != EOF) && !(isalpha(ch) || isdigit(ch)));

    if (ch == EOF)
        return NULL;
    
    do {
        word[i++] = tolower(ch);
    } while (((ch = fgetc(fp)) != EOF) && (isalpha(ch) || isdigit(ch)));

    word[i] = '\0';
    return strdup(word);
}

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

int parse_and (int argc, char *argv[])
{
    char **log_files = get_log_files();
    check(log_files != NULL, "Failed to get log files.");

    int i = 0;
    while (strcmp(log_files[i], ENDSTR) != 0) {
        int found = 1;
        for (int j = 1; j < argc; j++) {
            if (!is_in_file(argv[j], log_files[i])) {
                found = 0;
                break;
            }
        }

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

int parse_or (int argc, char *argv[])
{
    char **log_files = get_log_files();
    check(log_files != NULL, "Failed to get log files.");

    int i = 0;
    while (strcmp(log_files[i], ENDSTR) != 0) {
        int found = 0;
        for (int j = 1; j < argc; j++) {
            if (is_in_file(argv[j], log_files[i])) {
                found = 1;
                break;
            }
        }

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
    check(argc >= 2, "Usage: %s [-o] search-term-1 search-term-2 ...", argv[0]);

    check(argc != 2 || strcmp(argv[1], "-o") != 0, "Error: parameter -o requires at least one argument.");

    int rc;
    if (strcmp(argv[1], "-o") == 0) {
        rc = parse_or(argc, argv);
        check(rc != -1, "Couldn't parse arguments.");
    } else {
        rc = parse_and(argc, argv);
        check(rc != -1, "Couldn't parse arguments.");
    }

    return 0;

error:
    return -1;
}