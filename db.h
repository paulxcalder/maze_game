#ifndef DB_H
#define DB_H

#define DB_FILE         "scores.dat"
#define DB_TEMP         "scores.tmp"
#define DB_MAX_ENTRIES  256

typedef struct {
    char      name[33];
    float     time_sec;
    long long seed;
} DBEntry;

/*
 * Сохраняет результат пользователя.
 * Алгоритм:
 *   1) Читает основной файл; каждую строку, кроме строки целевого
 *      пользователя, копирует во временный файл.
 *      Строку целевого пользователя сохраняет в памяти.
 *   2) Перезаписывает основной файл содержимым временного.
 *   3) Дописывает запись пользователя (лучшее из старого и нового времени).
 *   4) Удаляет временный файл.
 */
void db_save(const char *name, float time_sec, long long seed);

/*
 * Загружает все записи из файла, сортирует по времени (меньше = лучше).
 * Возвращает количество загруженных записей (не более max_count).
 */
int  db_load(DBEntry *out, int max_count);

#endif /* DB_H */