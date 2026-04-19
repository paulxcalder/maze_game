#include "db.h"
#include "error.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* ─────────────────────────────────────────────────────────────────────────
 * db_save
 * Сохраняет результат игрока. Если игрок уже есть в базе — обновляет
 * его запись, оставляя лучшее (меньшее) время.
 * ───────────────────────────────────────────────────────────────────────── */
void db_save(const char *name, float time_sec, long long seed)
{
    /* ── Шаг 1: читаем основной файл ──────────────────────────────────── */
    FILE *main_f = fopen(DB_FILE, "r");
    FILE *temp_f = fopen(DB_TEMP, "w");

    if (!temp_f) {
        if (main_f) fclose(main_f);
        log_error("db_save: не удалось открыть временный файл");
        return;
    }

    /* Данные найденного пользователя (если есть) */
    char      found_name[33] = {0};
    float     found_time     = 0.0f;
    long long found_seed     = 0;
    int       found          = 0;

    if (main_f) {
        char      line_name[33];
        float     line_time;
        long long line_seed;

        while (fscanf(main_f, "%32s %f %lld",
                      line_name, &line_time, &line_seed) == 3) {
            if (strcmp(line_name, name) == 0) {
                /* Строку пользователя сохраняем в памяти, не пишем в temp */
                strncpy(found_name, line_name, 32);
                found_name[32] = '\0';
                found_time     = line_time;
                found_seed     = line_seed;
                found          = 1;
            } else {
                /* Остальные строки — копируем во временный файл */
                fprintf(temp_f, "%s %.3f %lld\n", line_name, line_time, line_seed);
            }
        }
        fclose(main_f);
    }
    fclose(temp_f);

    /* ── Шаг 2: перезаписываем основной файл содержимым временного ────── */
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

    /* ── Шаг 3: дописываем пользователя с лучшим временем ─────────────── */
    float     best_time = time_sec;
    long long best_seed = seed;

    if (found && found_time < time_sec) {
        /* Старое время лучше — сохраняем его */
        best_time = found_time;
        best_seed = found_seed;
    }

    fprintf(main_f, "%s %.3f %lld\n", name, best_time, best_seed);
    fclose(main_f);

    /* ── Шаг 4: удаляем временный файл ────────────────────────────────── */
    remove(DB_TEMP);
}

/* ─────────────────────────────────────────────────────────────────────────
 * db_load
 * Загружает записи из файла и сортирует по возрастанию времени.
 * ───────────────────────────────────────────────────────────────────────── */
int db_load(DBEntry *out, int max_count)
{
    FILE *f = fopen(DB_FILE, "r");
    if (!f) return 0;

    int count = 0;
    while (count < max_count &&
           fscanf(f, "%32s %f %lld",
                  out[count].name,
                  &out[count].time_sec,
                  &out[count].seed) == 3)
    {
        out[count].name[32] = '\0';
        count++;
    }
    fclose(f);

    /* Сортировка пузырьком: лучшее время (меньше) — первое */
    for (int i = 0; i < count - 1; i++) {
        for (int j = i + 1; j < count; j++) {
            if (out[j].time_sec < out[i].time_sec) {
                DBEntry tmp = out[i];
                out[i]      = out[j];
                out[j]      = tmp;
            }
        }
    }

    return count;
}