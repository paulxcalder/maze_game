#include "raylib.h"
#include <stdio.h>

#define FONT_PATH "DejaVuSansBold.ttf"

typedef enum { MENU, GAME, ABOUT } Screen;

int main(void)
{
    InitWindow(1920, 1080, "Maze Generator");
    SetTargetFPS(144);

    int cp[200], n = 0;
    for (int i = 32; i <= 126; i++) cp[n++] = i;
    for (int i = 1040; i <= 1103; i++) cp[n++] = i;
    cp[n++] = 1025; cp[n++] = 1105;
    Font font = LoadFontEx(FONT_PATH, 96, cp, n);
    SetTextureFilter(font.texture, TEXTURE_FILTER_BILINEAR);

    Screen screen = MENU;

    int width = 0, height = 0;
    int genW = 0, genH = 0;
    int active = -1;  //0 - ширина    1 - высота

    float scale = 20;

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground((Color){12, 12, 22, 255});

        if (screen == MENU)
        {
            // Заголовок
            Vector2 ts = MeasureTextEx(font, "MAZE GENERATOR", 88, 1);
            DrawTextEx(font, "MAZE GENERATOR",(Vector2){(1920 - ts.x) * 0.5f, 130}, 88, 1, WHITE);

            // Кнопки
            Rectangle btnStart  = {(1920 - 320) * 0.5f, 486, 320, 64};
            Rectangle btnAbout  = {(1920 - 320) * 0.5f, 582, 320, 64};
            Rectangle btnExit   = {(1920 - 320) * 0.5f, 678, 320, 64};

            Vector2 m = GetMousePosition();

            //старт
            int hotStart = CheckCollisionPointRec(m, btnStart);
            DrawRectangleRec(btnStart, hotStart ? (Color){15,55,140,255} : (Color){10,40,100,255});
            DrawRectangleLinesEx(btnStart, 2, hotStart ? WHITE : (Color){60,120,255,255});
            Vector2 st = MeasureTextEx(font, "СТАРТ", 28, 1);
            DrawTextEx(font, "СТАРТ",(Vector2){btnStart.x + (btnStart.width - st.x)*0.5f,btnStart.y + (btnStart.height - st.y)*0.5f}, 28, 1, WHITE);
            if (hotStart && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                screen = GAME;
                width = height = genW = genH = 0;
                active = -1;
            }

            //справка
            int hotAbout = CheckCollisionPointRec(m, btnAbout);
            DrawRectangleRec(btnAbout, hotAbout ? (Color){80,35,110,255} : (Color){60,25,85,255});
            DrawRectangleLinesEx(btnAbout, 2, hotAbout ? WHITE : (Color){170,80,255,255});
            Vector2 at = MeasureTextEx(font, "СПРАВКА", 28, 1);
            DrawTextEx(font, "СПРАВКА",(Vector2){btnAbout.x + (btnAbout.width - at.x)*0.5f,btnAbout.y + (btnAbout.height - at.y)*0.5f}, 28, 1, WHITE);
            if (hotAbout && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                screen = ABOUT;

            //выход
            int hotExit = CheckCollisionPointRec(m, btnExit);
            DrawRectangleRec(btnExit, hotExit ? (Color){120,20,20,255} : (Color){90,15,15,255});
            DrawRectangleLinesEx(btnExit, 2, hotExit ? WHITE : (Color){220,60,60,255});
            Vector2 et = MeasureTextEx(font, "ВЫХОД", 28, 1);
            DrawTextEx(font, "ВЫХОД",(Vector2){btnExit.x + (btnExit.width - et.x)*0.5f,btnExit.y + (btnExit.height - et.y)*0.5f}, 28, 1, WHITE);
            if (hotExit && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                break;
        }

        else if (screen == GAME)
        {
            Rectangle backBtn   = {20,  20,  170, 44};
            Rectangle widthBox  = {384, 238, 220, 52};
            Rectangle heightBox = {1152, 238, 220, 52};
            Rectangle genBtn    = {810, 410, 300, 62};

            Vector2 m = GetMousePosition();

            // Мышь
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                if (CheckCollisionPointRec(m, widthBox)) active = 0;
                else if (CheckCollisionPointRec(m, heightBox)) active = 1;
                else if (CheckCollisionPointRec(m, genBtn)) { genW = width; genH = height; }
                else if (CheckCollisionPointRec(m, backBtn)) screen = MENU;
                else active = -1;
            }

            // Клавиатура
            int key = GetCharPressed();
            while (key > 0)
            {
                if (key >= '0' && key <= '9')
                {
                    if (active == 0) width = width  * 10 + (key - '0');
                    if (active == 1) height = height * 10 + (key - '0');
                }
                key = GetCharPressed();
            }
            if (IsKeyPressed(KEY_BACKSPACE))
            {
                if (active == 0) width /= 10;
                if (active == 1) height /= 10;
            }

            // Кнопка < МЕНЮ
            int hotBack = CheckCollisionPointRec(m, backBtn);
            DrawRectangleRec(backBtn, hotBack ? (Color){55,55,68,255} : (Color){35,35,48,255});
            DrawRectangleLinesEx(backBtn, 2, GRAY);
            Vector2 bt = MeasureTextEx(font, "< МЕНЮ", 18, 1);
            DrawTextEx(font, "< МЕНЮ",(Vector2){backBtn.x + (backBtn.width - bt.x)*0.5f,backBtn.y + (backBtn.height - bt.y)*0.5f}, 18, 1, LIGHTGRAY);

            // Заголовок
            Vector2 title = MeasureTextEx(font, "НАСТРОЙКА ЛАБИРИНТА", 42, 1);
            DrawTextEx(font, "НАСТРОЙКА ЛАБИРИНТА",
                (Vector2){(1920 - title.x)*0.5f, 86}, 42, 1, WHITE);

            //текст
            DrawTextEx(font, "ШИРИНА",  (Vector2){widthBox.x,  widthBox.y  - 26}, 18, 1, LIGHTGRAY);
            DrawTextEx(font, "ВЫСОТА",  (Vector2){heightBox.x, heightBox.y - 26}, 18, 1, LIGHTGRAY);

            // Поля ввода
            DrawRectangleLinesEx(widthBox,  2, active == 0 ? BLUE : GRAY);
            DrawRectangleLinesEx(heightBox, 2, active == 1 ? BLUE : GRAY);
            DrawTextEx(font, TextFormat("%d", width),(Vector2){widthBox.x  + 12, widthBox.y  + 13}, 26, 1, GREEN);
            DrawTextEx(font, TextFormat("%d", height),(Vector2){heightBox.x + 12, heightBox.y + 13}, 26, 1, GREEN);

            //Генерация
            int hotGen = CheckCollisionPointRec(m, genBtn);
            DrawRectangleRec(genBtn, hotGen ? (Color){15,80,28,255} : (Color){8,55,18,255});
            DrawRectangleLinesEx(genBtn, 2, hotGen ? WHITE : (Color){50,200,80,255});
            Vector2 gt = MeasureTextEx(font, "СГЕНЕРИРОВАТЬ", 24, 1);
            DrawTextEx(font, "СГЕНЕРИРОВАТЬ",(Vector2){genBtn.x + (genBtn.width - gt.x)*0.5f,genBtn.y + (genBtn.height - gt.y)*0.5f}, 24, 1, WHITE);

            //отрисочка лабиринта
            if (genW > 0 && genH > 0)
            {
                DrawRectangleLines(960  - (int)(genW * scale) / 2,700  - (int)(genH * scale) / 2,(int)(genW * scale),(int)(genH * scale),WHITE);
                DrawTextEx(font, TextFormat("Размер: %d x %d", genW, genH),(Vector2){960 - (genW * scale) / 2,700 + (genH * scale) / 2 + 10},18, 1, LIGHTGRAY);
            }
        }

        else if (screen == ABOUT)
        {
            Rectangle backBtn = {20, 20, 170, 44};

            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(GetMousePosition(), backBtn))
                screen = MENU;

            Vector2 m = GetMousePosition();
            int hotBack = CheckCollisionPointRec(m, backBtn);
            DrawRectangleRec(backBtn, hotBack ? (Color){55,55,68,255} : (Color){35,35,48,255});
            DrawRectangleLinesEx(backBtn, 2, GRAY);
            Vector2 bt = MeasureTextEx(font, "< МЕНЮ", 18, 1);
            DrawTextEx(font, "< МЕНЮ",(Vector2){backBtn.x + (backBtn.width - bt.x)*0.5f,backBtn.y + (backBtn.height - bt.y)*0.5f}, 18, 1, LIGHTGRAY);

            Vector2 ts = MeasureTextEx(font, "СПРАВКА", 52, 1);
            DrawTextEx(font, "СПРАВКА",(Vector2){(1920 - ts.x)*0.5f, 76}, 52, 1, WHITE);
            DrawRectangle(1920/2 - 280, 194, 560, 2, (Color){80,80,110,255});

            //отрисовка

            DrawTextEx(font,"Авторы:",(Vector2){1920 * 0.15f, 248},22, 1, GRAY);
            DrawTextEx(font,"Абдульманов Данияр Айдарович",(Vector2){1920 * 0.15f + 200, 248},22, 1, WHITE);
            DrawTextEx(font,"Сергунин Роман Дмитриевич",(Vector2){1920 * 0.15f + 200, 284}, 22, 1, WHITE);

            DrawTextEx(font,"Группа:",(Vector2){1920 * 0.15f, 338}, 22, 1, GRAY);
            DrawTextEx(font,"5131001/50601",(Vector2){1920 * 0.15f + 200, 338},22, 1, WHITE);

            DrawTextEx(font,"Год:",(Vector2){1920 * 0.15f, 392}, 22, 1, GRAY);
            DrawTextEx(font,"2026",(Vector2){1920 * 0.15f + 200, 392}, 22, 1, WHITE);

            DrawTextEx(font,"ВУЗ:",(Vector2){1920 * 0.15f, 446}, 22, 1, GRAY);
            DrawTextEx(font,"СПбГУ им. Петра Великого", (Vector2){1920 * 0.15f + 200, 446}, 22, 1, WHITE);

            DrawTextEx(font,"Институт:",(Vector2){1920 * 0.15f, 500}, 22, 1, GRAY);
            DrawTextEx(font,"Институт компьютерных наук и кибербезопасности",(Vector2){1920 * 0.15f + 200, 500}, 22, 1, WHITE);

            DrawTextEx(font,"Кафедра:",(Vector2){1920 * 0.15f, 554}, 22, 1, GRAY);
            DrawTextEx(font,"Высшая Школа Кибербезопасности",(Vector2){1920 * 0.15f + 200, 554}, 22, 1, WHITE);
        }
        EndDrawing();
    }

    UnloadFont(font);
    CloseWindow();
    return 0;
}