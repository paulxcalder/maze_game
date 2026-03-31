#ifndef LABYRINTH_H
#define LABYRINTH_H

typedef unsigned short size_l;

extern char **map;
extern size_l width;
extern size_l height;
extern size_l x_end;
extern size_l y_end;

char generate_lab();
char find_way(size_l x, size_l y);
void print_lab();
void free_lab();

#endif