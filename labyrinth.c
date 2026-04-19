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
    char dir, need_cell = 0;

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

    do{
        for(size_l i = 0; i < height; i++){
            for(size_l j = 0; j < width; j++){
                if(i == 0 || i == height - 1 || j == 0 || j == width - 1){
                    map[i][j] = WALL;
                }
                else if(((i + j) & 1) == 0 && need_cell){
                    map[i][j] = WAY;

                    if(i == 1 && j == width - 2){
                        continue;
                    }

                    dir = rand() & 1;
                    if(dir){
                        dir -= (j+2 >= width);
                    }
                    else{
                        dir += (i == 1);
                    }
                    map[i - (1-dir)][j + dir] = WAY;
                    j += dir;             
                }
                else{
                    if(i == height - 3 && (rand() & 1)){
                        map[i][j] = WAY;
                    }
                    else{
                        map[i][j] = WALL;
                    }
                }
            }
            need_cell = (need_cell + 1) & 1;
        }

        need_cell = 1;
        for(size_l j = 1; j < width - 1; j++){
            if(map[height - 2][j] == WAY){
                map[height - 2][j] = END;
                need_cell = 0;
                break;
            }
        }
        if(need_cell){
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