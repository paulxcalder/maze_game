#include "graphic.h"
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "labyrinth.h"

int main(void)
{
    srand(time(NULL));
    RunMazeGenerator();
    return 0;
}