#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef unsigned short size_l;

char **map = NULL; //двумерный список
size_l width = 0; //ширина
size_l height = 0; //длина
size_l x_end, y_end; //конечные точки
char wall = '@'; 
char way = ' ';
char right_way = '#';
char visited = 'v';

void free_lab(){ //для освобождения 
    if(map != NULL){
        for(size_l i = 0; i < height; i++){
            if(map[i] != NULL){ //освобождаем построчно
                free(map[i]);
            }
        }
        free(map); //полностью высвобождаем память
        map = NULL;
    }
}

char find_way(size_l x, size_l y){ //поиск пути(до конечной координаты)
    if(map[y][x] == wall || map[y][x] == visited){ //если на позиции стенка или мы уже там были
        return 0;
    }
    else if(map[y][x] == right_way){ //если мы пришли в конечную
        return 1;
    }

    map[y][x] = visited; //ставим клетку как пройденную 
    if(find_way(x + 1, y) || find_way(x, y + 1) || find_way(x - 1, y) || find_way(x, y - 1)){ //рекурсивно если у нас есть куда сходить и мы добрались до конечной,  
        map[y][x] = right_way; //то клетка ведет к выходу и мы все остальные помечаем красной решеткой
        return 1;
    }
    map[y][x] = way; //вернули нормальную, не пришли к выходу
    return 0;
}

char generate_lab(){ //генерация лабиринта
    if(height <= 2 || width <= 2){ //если размер маленький
        printf("Incorrect size!\n");
        return 0;
    }

    map = (char**)malloc(height * sizeof(char*)); 
    if(map == NULL){
        printf("Memory allocation error!\n");//если не выделилась память под сам массив
        return -1;
    }

    char flag = 0; //для чередования
    for(size_l i = 0; i < height; i++){
        map[i] = (char*)malloc(width * sizeof(char));
        if(map[i] == NULL){ //если вдруг на какой-то строке не выделилась память
            printf("Incorrect size!\n");
            free_lab();
            return -1;
        }

        for(size_l j = 0; j < width; j++){
            if(i == 0 || i == height - 1 || j == 0 || j == width - 1){ //генериуем коробку из символов @
                map[i][j] = wall;
            }
            else if(((i + j) & 1) == 0 && flag){ //если четный номер и флаг (типо 1+1  или 3 + 1) втавим пустоту +
                map[i][j] = way;
            }
            else{
                map[i][j] = wall; //иначе стена (заполняем через одину штуку)
            }
        }
        flag = (flag + 1) & 1; // 0 + 1 & 1 == 1            1+1 & 1 == 0
    }

    for(size_l i = 1; i < height - 1; i += 2){ //прыгаем по пустым ячейкам
        for(size_l j = 1; j < width - 1; j += 2){
            flag = 1; //если 1 - не пробили направление, иначе не пробили
            while(flag){
                if(i == 1 && j == width - 2){ //угловые клетки (ширина нечетная - не можем пробить право вверх)
                    flag = 0;
                }
                else if((rand() & 1) == 1){ //1 - направо     0 - наверх//пытаемся пробить направо, и можно ди вообще
                    if(j + 2 < width){ //если мы не стоим в угловой клетке 
                        map[i][j+1] = way; //рисуем ' ' 
                        flag = 0;
                    }
                }
                else{ //если наверх
                    if(i > 1){ //если есть куда наверх
                        map[i-1][j] = way; //  ставим пробел
                        flag = 0;
                    }
                }//пробиваемся вниз 
                if(i == height - 3  && (rand() & 1) == 1){ //если в поселдней строке четного лабиринта
                    map[i+1][j] = way;
                }
            }
        }
    }

    flag = 1;
    for(size_l j = 1; j < width - 1; j++){ //проходимся столбикам
        if(map[height - 2][j] == way){ //если мы нашли пустую клетку в нижнйе строке
            map[height - 2][j] = right_way; //делаем ее выходом
            x_end = j;
            y_end = height - 2;
            flag = 0;
            break;
        }
    }
    if(flag){ //если нигде нет пустой клетки, то сами ее прорубаем
        map[height - 2][1] = right_way;
        x_end = 1;
        y_end = height - 2;
    }
    return 1;
}

void print_lab(){
    for(size_l i = 0; i < height; i++){
        for(size_l j = 0; j < width; j++){
            if((j == x_end && i == y_end) || (i == 1 && j == 1)){ //если у нас начало или конец
                printf("\033[32m%c\033[0m", map[i][j]); //выводим зеленым
            }
            else if(map[i][j] == right_way){ //если это путь
                printf("\033[31m%c\033[0m", map[i][j]); //выводим краснным
            }
            else{
                printf("%c", map[i][j]); //иначе просто выводим белым
            }
        }
        printf("\n");//для новой строки
    }
}

int main(){
    srand(time(NULL));//генератор случайных чисел

    printf("Height: "); //запрос высоты
    scanf("%hu", &height);
    
    printf("Width: "); //запрос ширины
    scanf("%hu", &width);   
    
    generate_lab(); //запрос генерируем лабиринт
    while(!find_way(1, 1)){ //если мы не нашли выход
        free_lab(); //очищаем и генерируем заново
        generate_lab();
    }
    print_lab(); //печатаем
    return 0;
}