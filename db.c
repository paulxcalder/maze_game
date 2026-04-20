#include "db.h"
#include "error.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void db_save(const char *name, float time_sec, long long seed, int width, int height)
{
    FILE *main_f = fopen(DB_FILE, "r");
    FILE *temp_f = fopen(DB_TEMP, "w");

    if (!temp_f) {
        if (main_f) fclose(main_f);
        log_error("db_save: не удалось открыть временный файл");
        return;
    }

    char found_name[33] = {0};
    float found_time = 0.0f;
    long long found_seed = 0;
    int found_w = 0, found_h = 0;
    int found = 0;

    if (main_f) {
        char line_name[33];
        float line_time;
        long long line_seed;
        int line_w, line_h;

        while (fscanf(main_f, "%32s %f %lld %d %d",
                      line_name, &line_time, &line_seed, &line_w, &line_h) == 5) {
            if (strcmp(line_name, name) == 0) {
                strncpy(found_name, line_name, 32);
                found_name[32] = '\0';
                found_time = line_time;
                found_seed = line_seed;
                found_w = line_w;
                found_h = line_h;
                found = 1;
            } else {
                fprintf(temp_f, "%s %.3f %lld %d %d\n",
                        line_name, line_time, line_seed, line_w, line_h);
            }
        }
        fclose(main_f);
    }
    fclose(temp_f);

    temp_f = fopen(DB_TEMP, "r");
    main_f = fopen(DB_FILE, "w");

    if (!main_f) {
        if (temp_f) fclose(temp_f);
        remove(DB_TEMP);
        log_error("db_save: не удалось открыть основной файл для записи");
        return;
    }

    if (temp_f) {
        char buf[256];
        while (fgets(buf, sizeof(buf), temp_f))
            fputs(buf, main_f);
        fclose(temp_f);
    }

    float best_time = time_sec;
    long long best_seed = seed;
    int best_w = width, best_h = height;

    if (found && found_time < time_sec) {
        best_time = found_time;
        best_seed = found_seed;
        best_w = found_w;
        best_h = found_h;
    }

    fprintf(main_f, "%s %.3f %lld %d %d\n", name, best_time, best_seed, best_w, best_h);
    fclose(main_f);
    remove(DB_TEMP);
}

int db_load(DBEntry *out, int max_count)
{
    FILE *f = fopen(DB_FILE, "r");
    if (!f) return 0;

    int count = 0;
    while (count < max_count &&
           fscanf(f, "%32s %f %lld %d %d",
                  out[count].name, &out[count].time_sec,
                  &out[count].seed, &out[count].width, &out[count].height) == 5)
    {
        out[count].name[32] = '\0';
        count++;
    }
    fclose(f);

    //сортировка по времени
    for (int i = 0; i < count - 1; i++)
        for (int j = i + 1; j < count; j++)
            if (out[j].time_sec < out[i].time_sec) {
                DBEntry tmp = out[i]; out[i] = out[j]; out[j] = tmp;
            }

    return count;
}