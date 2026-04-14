#include "error.h"
#include <stdio.h>
#include <time.h>

#define LOG_FILE "maze.log"

static FILE *log_file = NULL;

static void log_open(void)
{
    if (log_file == NULL)
        log_file = fopen(LOG_FILE, "a");
}

static void log_write(const char *level, const char *msg)
{
    log_open();
    if (log_file == NULL) return;

    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    fprintf(log_file, "[%04d-%02d-%02d %02d:%02d:%02d] [%s] %s\n",
        t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
        t->tm_hour, t->tm_min, t->tm_sec,
        level, msg);

    fflush(log_file);
}

void log_info(const char *msg)  { log_write("INFO",  msg); }
void log_error(const char *msg) { log_write("ERROR", msg); }

void log_close(void)
{
    if (log_file != NULL)
    {
        fclose(log_file);
        log_file = NULL;
    }
}