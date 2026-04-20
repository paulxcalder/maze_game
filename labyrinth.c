#include <stdlib.h>
#include <time.h>
#include "labyrinth.h"

#define VISITED 'V'

static int _malloc_count = 0;
static int _realloc_count = 0;
static int _calloc_count = 0;
static int _free_count = 0;

void free_lab(char **map, size_l height){
    if(map != NULL){
        for(size_l i = 0; i < height; i++){
            if(map[i] != NULL){
                free(map[i]);
                _free_count++;
            }
        }
        free(map);
        _free_count++;
    }
}

char find_way(char **map, size_l cur_x, size_l cur_y, char show){
    if(map[cur_y][cur_x] == WALL || map[cur_y][cur_x] == VISITED){
        return 0;
    }
    else if(map[cur_y][cur_x] == END){
        return 1;
    }

    map[cur_y][cur_x] = VISITED;
    if(find_way(map, cur_x + 1, cur_y, show) ||
       find_way(map, cur_x, cur_y + 1, show) ||
       find_way(map, cur_x - 1, cur_y, show) ||
       find_way(map, cur_x, cur_y - 1, show)){
        if(show){
            map[cur_y][cur_x] = RIGHT_WAY;
        }
        else{
            map[cur_y][cur_x] = WAY;
        }
        return 1;
    }
    map[cur_y][cur_x] = WAY;
    return 0;
}

char **generate_lab(long long seed, size_l width, size_l height){
    if(seed == -1){
        seed = time(NULL);
    }
    srand(seed);

    if(height <= 2 || width <= 2){
        return NULL;
    }

    char **map = (char**)malloc(height * sizeof(char*));
    _malloc_count++;

    if(map == NULL){
        return NULL;
    }

    for(size_l i = 0; i < height; i++){
        map[i] = (char*)malloc(width * sizeof(char));
        _malloc_count++;

        if(map[i] == NULL){
            free_lab(map, height);
            return NULL;
        }
    }

    char cell_row;
    do{
        //рамка + сетка ячеек (узлы будущего лабиринта стоят через клетку)
        cell_row = 0;
        for(size_l i = 0; i < height; i++){
            for(size_l j = 0; j < width; j++){
                if(i == 0 || i == height - 1 || j == 0 || j == width - 1){
                    map[i][j] = WALL;
                }
                else if(((i + j) & 1) == 0 && cell_row){
                    map[i][j] = WAY;
                }
                else{
                    map[i][j] = WALL;
                }
            }
            cell_row = !cell_row;
        }

        //пробивка проходов между соседними ячейками (вправо или вверх)
        for(size_l i = 1; i < height - 1; i += 2){
            for(size_l j = 1; j < width - 1; j += 2){
                char need_link = 1;
                while(need_link){
                    if(i == 1 && j == width - 2){
                        need_link = 0;
                    }
                    else if(rand() & 1){
                        if(j + 2 < width || (width & 1) == 0){
                            map[i][j + 1] = WAY;
                            need_link = 0;
                        }
                    }
                    else{
                        if(i > 1){
                            map[i - 1][j] = WAY;
                            need_link = 0;
                        }
                    }
                    if(i == height - 3 && (height & 1) == 0 && (rand() & 1)){
                        map[i + 1][j] = WAY;
                    }
                }
            }
        }

        //выход на нижней границе
        char no_end = 1;
        for(size_l j = 1; j < width - 1; j++){
            if(map[height - 2][j] == WAY){
                map[height - 2][j] = END;
                no_end = 0;
                break;
            }
        }
        if(no_end){
            map[height - 2][1] = END;
        }
    }while(!find_way(map, 1, 1, 0));

    return map;
}

void get_memstat(int *malloc_count, int *realloc_count, int *calloc_count, int *free_count){
    *malloc_count = _malloc_count;
    *realloc_count = _realloc_count;
    *calloc_count = _calloc_count;
    *free_count = _free_count;
}