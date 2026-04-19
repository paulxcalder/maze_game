#ifndef LABYRINTH_H
#define LABYRINTH_H

#define WALL '@'
#define WAY ' '
#define RIGHT_WAY '#'
#define END '!'

typedef unsigned short size_l;

void free_lab(char **map, size_l height);
char find_way(char **map, size_l cur_x, size_l cur_y, char show);
char **generate_lab(long long seed, size_l width, size_l height);
void get_memstat(int *malloc_count, int *realloc_count, int *calloc_count, int *free_count);

#endif