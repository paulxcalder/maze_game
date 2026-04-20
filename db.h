#ifndef DB_H
#define DB_H

#define DB_FILE "scores.dat"
#define DB_TEMP "scores.tmp"
#define DB_MAX_ENTRIES 256

typedef struct {
    char name[33];
    float time_sec;
    long long seed;
    int width;
    int height;
} DBEntry;

void db_save(const char *name, float time_sec, long long seed, int width, int height);
int  db_load(DBEntry *out, int max_count);

#endif /* DB_H */