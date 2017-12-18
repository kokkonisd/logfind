#ifndef __logfind_h__
#define __logfind_h__

#include <stdio.h>
#include <errno.h>
#include <string.h>

#define clean_errno() (errno == 0 ? "None" : strerror(errno))

#define log_err(M, ...) fprintf(stderr,\
        "[logfind] (%s:%s:%d: errno: %s) " M "\n", __FUNCTION__, __FILE__,\
        __LINE__, clean_errno(), ##__VA_ARGS__)

#define check(A, M, ...) if(!(A)) {\
    log_err(M, ##__VA_ARGS__); errno=0; goto error; }
#endif

#define ENDSTR "END_OF_LOG_FILES"
#define LOGFILE_LIST "~/.logfind"
#define MAX_WORD_SIZE 1024
#define MAX_PATH_SIZE (MAX_WORD_SIZE * 2)
#define VERSION "1.1"
