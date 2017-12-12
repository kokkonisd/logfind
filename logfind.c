/*
Written by Dimitris Kokkonis

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
#define LOGS "/var/log/"
#define MAX_WORD_SIZE 1024

char **get_log_files (char *logdir)
{
    DIR *dir;
    struct dirent *ent;
    int dircount = 0;
    int max_name_len = 0;


    dir = opendir(logdir);
    check(dir != NULL, "Failed to open log directory.");
    
    while ((ent = readdir(dir)) != NULL) {
        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
            continue;

        dircount++;

        if (strlen(ent->d_name) > max_name_len)
            max_name_len = strlen(ent->d_name);
    }

    closedir(dir);

    char final_name[1000];

    char **logfiles = malloc((dircount + 1) * sizeof(char *));
    check(logfiles != NULL, "Memory error.");

    dir = opendir(logdir);
    check(dir != NULL, "Failed to open log directory.");
    for (int i = 0; i < dircount; i++) {
        logfiles[i] = malloc((max_name_len + strlen(logdir) + 1) * sizeof(char));
        check(logfiles[i] != NULL, "Memory error.");

        ent = readdir(dir);

        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) {
            i--;
            continue;
        }

        strcpy(final_name, logdir);
        strcat(final_name, ent->d_name);
        strcpy(logfiles[i], final_name);
    }

    logfiles[dircount] = ENDSTR;

    closedir(dir);

    return logfiles;

error:
    return NULL;
}

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

char *get_word (FILE *fp) {
    char word[MAX_WORD_SIZE];
    int ch, i = 0;

    while (EOF != (ch = fgetc(fp)) && !(isalpha(ch) || isdigit(ch))); // skip

    if (ch == EOF)
        return NULL;
    
    do {
        word[i++] = tolower(ch);
    } while (EOF != (ch = fgetc(fp)) && (isalpha(ch) || isdigit(ch)));

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
    char **log_files = get_log_files(LOGS);
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
    char **log_files = get_log_files(LOGS);
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