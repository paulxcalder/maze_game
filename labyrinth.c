#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "labyrinth.h"
typedef unsigned short size_l;

char **map = NULL;
size_l width = 0;
size_l height = 0;
size_l x_end, y_end;
char wall = '@';
char way = ' ';
char right_way = '#';
char visited ='v';

void free_lab(){
    if(map != NULL){
        for(size_l i = 0; i < height; i++){
            if(map[i] != NULL){
                free(map[i]);
            }
        }
        free(map);
        map = NULL;
    }
}


void reset_path_marks(){
    if(map == NULL) return;
    for(size_l i = 0; i < height; i++){
        for(size_l j = 0; j < width; j++){
            if(map[i][j] == right_way){
                map[i][j] = way;
            }
        }
    }
}

char find_way(size_l x, size_l y){
    if(map[y][x] == wall || map[y][x] == visited){
        return 0;
    }
    else if(map[y][x] == right_way){
        return 1;
    }

    map[y][x] = visited;
    if(find_way(x + 1, y) || find_way(x, y + 1) || find_way(x - 1, y) || find_way(x, y - 1)){
        map[y][x] = right_way;
        return 1;
    }
    map[y][x] = way;
    return 0;
}

char generate_lab(){
    if(height <= 2 || width <= 2){
        printf("Incorrect size!\n");
        return 0;
    }

    map = (char**)malloc(height * sizeof(char*));
    if(map == NULL){
        printf("Memory allocation error!\n");
        return -1;
    }

    char flag = 0;
    for(size_l i = 0; i < height; i++){
        map[i] = (char*)malloc(width * sizeof(char));
        if(map[i] == NULL){
            printf("Incorrect size!\n");
            free_lab();
            return -1;
        }

        for(size_l j = 0; j < width; j++){
            if(i == 0 || i == height - 1 || j == 0 || j == width - 1){
                map[i][j] = wall;
            }
            else if(((i + j) & 1) == 0 && flag){
                map[i][j] = way;
            }
            else{
                map[i][j] = wall;
            }
        }
        flag = (flag + 1) & 1;
    }

    for(size_l i = 1; i < height - 1; i += 2){
        for(size_l j = 1; j < width - 1; j += 2){
            flag = 1;
            while(flag){
                if(i == 1 && j == width - 2){
                    flag = 0;
                }
                else if((rand() & 1) == 1){
                    if(j + 2 < width){
                        map[i][j+1] = way;
                        flag = 0;
                    }
                }
                else{
                    if(i > 1){
                        map[i-1][j] = way;
                        flag = 0;
                    }
                }
                if(i == height - 3 && (rand() & 1) == 1){
                    map[i+1][j] = way;
                }
            }
        }
    }

    flag = 1;
    for(size_l j = 1; j < width - 1; j++){
        if(map[height - 2][j] == way){
            map[height - 2][j] = right_way;
            x_end = j;
            y_end = height - 2;
            flag = 0;
            break;
        }
    }
    if(flag){
        map[height - 2][1] = right_way;
        x_end = 1;
        y_end = height - 2;
    }
    return 1;
}

void print_lab(){
    for(size_l i = 0; i < height; i++){
        for(size_l j = 0; j < width; j++){
            if((j == x_end && i == y_end) || (i == 1 && j == 1)){
                printf("\033[32m%c\033[0m", map[i][j]);
            }
            else if(map[i][j] == right_way){
                printf("\033[31m%c\033[0m", map[i][j]);
            }
            else{
                printf("%c", map[i][j]);
            }
        }
        printf("\n");
    }
}