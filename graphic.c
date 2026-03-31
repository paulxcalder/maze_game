#include "graphic.h"
#include "raylib.h"
#include <stdio.h>

#define FONT_PATH "DejaVuSansBold.ttf"

typedef enum {
    MENU,
    MODE_SELECT,
    GAME,
    ABOUT,
    HELP,
    BOT_VS_BOT,
    KEY_INPUT
} Screen;

static void DrawBackButton(Font font, Vector2 m, Rectangle backBtn, const char *label, Screen *screen, Screen target)
{
    int hotBack = CheckCollisionPointRec(m, backBtn);
    DrawRectangleRec(backBtn, hotBack ? (Color){55,55,68,255} : (Color){35,35,48,255});
    DrawRectangleLinesEx(backBtn, 2, GRAY);

    Vector2 bt = MeasureTextEx(font, label, 18, 1);
    DrawTextEx(font, label,
               (Vector2){backBtn.x + (backBtn.width - bt.x) * 0.5f,
                         backBtn.y + (backBtn.height - bt.y) * 0.5f},
               18, 1, LIGHTGRAY);

    if (hotBack && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        *screen = target;
}

static void DrawMenu(Font font, Screen *screen)
{
    Vector2 ts = MeasureTextEx(font, "MAZE GENERATOR", 88, 1);
    DrawTextEx(font, "MAZE GENERATOR", (Vector2){(1920 - ts.x) * 0.5f, 110}, 88, 1, WHITE);

    Rectangle btnStart = {(1920 - 320) * 0.5f, 400, 320, 64};
    Rectangle btnHelp   = {(1920 - 320) * 0.5f, 490, 320, 64};
    Rectangle btnAbout  = {(1920 - 320) * 0.5f, 580, 320, 64};
    Rectangle btnExit   = {(1920 - 320) * 0.5f, 670, 320, 64};

    Vector2 m = GetMousePosition();

    int hotStart = CheckCollisionPointRec(m, btnStart);
    DrawRectangleRec(btnStart, hotStart ? (Color){15,55,140,255} : (Color){10,40,100,255});
    DrawRectangleLinesEx(btnStart, 2, hotStart ? WHITE : (Color){60,120,255,255});
    Vector2 st = MeasureTextEx(font, "ИГРАТЬ", 28, 1);
    DrawTextEx(font, "ИГРАТЬ",
               (Vector2){btnStart.x + (btnStart.width - st.x) * 0.5f,
                         btnStart.y + (btnStart.height - st.y) * 0.5f},
               28, 1, WHITE);
    if (hotStart && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        *screen = MODE_SELECT;

    int hotHelp = CheckCollisionPointRec(m, btnHelp);
    DrawRectangleRec(btnHelp, hotHelp ? (Color){20,80,90,255} : (Color){14,58,68,255});
    DrawRectangleLinesEx(btnHelp, 2, hotHelp ? WHITE : (Color){40,190,210,255});
    Vector2 ht = MeasureTextEx(font, "ПОМОЩЬ", 28, 1);
    DrawTextEx(font, "ПОМОЩЬ",
               (Vector2){btnHelp.x + (btnHelp.width - ht.x) * 0.5f,
                         btnHelp.y + (btnHelp.height - ht.y) * 0.5f},
               28, 1, WHITE);
    if (hotHelp && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        *screen = HELP;

    int hotAbout = CheckCollisionPointRec(m, btnAbout);
    DrawRectangleRec(btnAbout, hotAbout ? (Color){80,35,110,255} : (Color){60,25,85,255});
    DrawRectangleLinesEx(btnAbout, 2, hotAbout ? WHITE : (Color){170,80,255,255});
    Vector2 at = MeasureTextEx(font, "СПРАВКА", 28, 1);
    DrawTextEx(font, "СПРАВКА",
               (Vector2){btnAbout.x + (btnAbout.width - at.x) * 0.5f,
                         btnAbout.y + (btnAbout.height - at.y) * 0.5f},
               28, 1, WHITE);
    if (hotAbout && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        *screen = ABOUT;

    int hotExit = CheckCollisionPointRec(m, btnExit);
    DrawRectangleRec(btnExit, hotExit ? (Color){120,20,20,255} : (Color){90,15,15,255});
    DrawRectangleLinesEx(btnExit, 2, hotExit ? WHITE : (Color){220,60,60,255});
    Vector2 et = MeasureTextEx(font, "ВЫХОД", 28, 1);
    DrawTextEx(font, "ВЫХОД",
               (Vector2){btnExit.x + (btnExit.width - et.x) * 0.5f,
                         btnExit.y + (btnExit.height - et.y) * 0.5f},
               28, 1, WHITE);
    if (hotExit && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        CloseWindow();
}

static void DrawModeSelect(Font font, Screen *screen)
{
    Rectangle backBtn = {20, 20, 170, 44};
    Rectangle btnSolo = {(1920 - 400) * 0.5f, 380, 400, 70};
    Rectangle btnBotBot = {(1920 - 400) * 0.5f, 480, 400, 70};
    Rectangle btnKey = {(1920 - 400) * 0.5f, 580, 400, 70};

    Vector2 m = GetMousePosition();

    Vector2 title = MeasureTextEx(font, "ВЫБОР РЕЖИМА", 52, 1);
    DrawTextEx(font, "ВЫБОР РЕЖИМА", (Vector2){(1920 - title.x) * 0.5f, 200}, 52, 1, WHITE);
    DrawRectangle(1920 / 2 - 300, 270, 600, 2, (Color){60,60,90,255});

    DrawBackButton(font, m, backBtn, "< МЕНЮ", screen, MENU);

    int hotSolo = CheckCollisionPointRec(m, btnSolo);
    DrawRectangleRec(btnSolo, hotSolo ? (Color){15,55,140,255} : (Color){10,40,100,255});
    DrawRectangleLinesEx(btnSolo, 2, hotSolo ? WHITE : (Color){60,120,255,255});
    Vector2 s1 = MeasureTextEx(font, "В ОДИНОЧКУ", 28, 1);
    DrawTextEx(font, "В ОДИНОЧКУ",
               (Vector2){btnSolo.x + (btnSolo.width - s1.x) * 0.5f,
                         btnSolo.y + (btnSolo.height - s1.y) * 0.5f},
               28, 1, WHITE);
    if (hotSolo && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        *screen = GAME;

    int hotBotBot = CheckCollisionPointRec(m, btnBotBot);
    DrawRectangleRec(btnBotBot, hotBotBot ? (Color){80,35,110,255} : (Color){60,25,85,255});
    DrawRectangleLinesEx(btnBotBot, 2, hotBotBot ? WHITE : (Color){170,80,255,255});
    Vector2 s2 = MeasureTextEx(font, "БОТ ПРОТИВ БОТА", 28, 1);
    DrawTextEx(font, "БОТ ПРОТИВ БОТА",
               (Vector2){btnBotBot.x + (btnBotBot.width - s2.x) * 0.5f,
                         btnBotBot.y + (btnBotBot.height - s2.y) * 0.5f},
               28, 1, WHITE);
    if (hotBotBot && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        *screen = BOT_VS_BOT;

    int hotKey = CheckCollisionPointRec(m, btnKey);
    DrawRectangleRec(btnKey, hotKey ? (Color){20,80,90,255} : (Color){14,58,68,255});
    DrawRectangleLinesEx(btnKey, 2, hotKey ? WHITE : (Color){40,190,210,255});
    Vector2 s3 = MeasureTextEx(font, "ВВЕСТИ КЛЮЧ", 28, 1);
    DrawTextEx(font, "ВВЕСТИ КЛЮЧ",
               (Vector2){btnKey.x + (btnKey.width - s3.x) * 0.5f,
                         btnKey.y + (btnKey.height - s3.y) * 0.5f},
               28, 1, WHITE);
    if (hotKey && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        *screen = KEY_INPUT;
}

static void DrawGame(Font font, Screen *screen, int *width, int *height, int *genW, int *genH, int *active)
{
    Rectangle backBtn   = {20,  20,  170, 44};
    Rectangle widthBox  = {384, 238, 220, 52};
    Rectangle heightBox = {1152, 238, 220, 52};
    Rectangle genBtn    = {810, 410, 300, 62};

    Vector2 m = GetMousePosition();

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        if (CheckCollisionPointRec(m, widthBox)) *active = 0;
        else if (CheckCollisionPointRec(m, heightBox)) *active = 1;
        else if (CheckCollisionPointRec(m, genBtn)) { *genW = *width; *genH = *height; }
        else if (CheckCollisionPointRec(m, backBtn)) *screen = MODE_SELECT;
        else *active = -1;
    }

    int key = GetCharPressed();
    while (key > 0)
    {
        if (key >= '0' && key <= '9')
        {
            if (*active == 0) {
                int v = (*width) * 10 + (key - '0');
                *width = (v > 100) ? 0 : v;
            }
            if (*active == 1) {
                int v = (*height) * 10 + (key - '0');
                *height = (v > 100) ? 0 : v;
            }
        }
        key = GetCharPressed();
    }

    if (IsKeyPressed(KEY_BACKSPACE))
    {
        if (*active == 0) *width /= 10;
        if (*active == 1) *height /= 10;
    }

    DrawBackButton(font, m, backBtn, "< НАЗАД", screen, MODE_SELECT);

    Vector2 title = MeasureTextEx(font, "НАСТРОЙКА ЛАБИРИНТА", 42, 1);
    DrawTextEx(font, "НАСТРОЙКА ЛАБИРИНТА",
               (Vector2){(1920 - title.x) * 0.5f, 86}, 42, 1, WHITE);

    DrawTextEx(font, "ШИРИНА", (Vector2){widthBox.x,  widthBox.y  - 26}, 18, 1, LIGHTGRAY);
    DrawTextEx(font, "ВЫСОТА",  (Vector2){heightBox.x, heightBox.y - 26}, 18, 1, LIGHTGRAY);
    DrawTextEx(font, "(макс. 100)", (Vector2){widthBox.x,  widthBox.y  + 58}, 16, 1, (Color){100,100,130,255});
    DrawTextEx(font, "(макс. 100)", (Vector2){heightBox.x, heightBox.y + 58}, 16, 1, (Color){100,100,130,255});

    DrawRectangleLinesEx(widthBox, 2, *active == 0 ? BLUE : GRAY);
    DrawRectangleLinesEx(heightBox, 2, *active == 1 ? BLUE : GRAY);
    DrawTextEx(font, TextFormat("%d", *width),  (Vector2){widthBox.x + 12,  widthBox.y  + 13}, 26, 1, GREEN);
    DrawTextEx(font, TextFormat("%d", *height), (Vector2){heightBox.x + 12, heightBox.y + 13}, 26, 1, GREEN);

    int hotGen = CheckCollisionPointRec(m, genBtn);
    DrawRectangleRec(genBtn, hotGen ? (Color){15,80,28,255} : (Color){8,55,18,255});
    DrawRectangleLinesEx(genBtn, 2, hotGen ? WHITE : (Color){50,200,80,255});
    Vector2 gt = MeasureTextEx(font, "СГЕНЕРИРОВАТЬ", 24, 1);
    DrawTextEx(font, "СГЕНЕРИРОВАТЬ",
               (Vector2){genBtn.x + (genBtn.width - gt.x) * 0.5f,
                         genBtn.y + (genBtn.height - gt.y) * 0.5f},
               24, 1, WHITE);

    if (*genW > 0 && *genH > 0)
    {
        float scale = 20;
        DrawRectangleLines(
            960 - (int)(*genW * scale) / 2,
            700 - (int)(*genH * scale) / 2,
            (int)(*genW * scale),
            (int)(*genH * scale),
            WHITE);

        DrawTextEx(font, TextFormat("Размер: %d x %d", *genW, *genH),
                   (Vector2){960 - (*genW * scale) / 2, 700 + (*genH * scale) / 2 + 10},
                   18, 1, LIGHTGRAY);
    }
}

static void DrawAbout(Font font, Screen *screen)
{
    Rectangle backBtn = {20, 20, 170, 44};
    Vector2 m = GetMousePosition();

    DrawBackButton(font, m, backBtn, "< МЕНЮ", screen, MENU);

    Vector2 ts = MeasureTextEx(font, "СПРАВКА", 52, 1);
    DrawTextEx(font, "СПРАВКА", (Vector2){(1920 - ts.x) * 0.5f, 76}, 52, 1, WHITE);
    DrawRectangle(1920 / 2 - 280, 194, 560, 2, (Color){80,80,110,255});

    DrawTextEx(font, "Авторы:", (Vector2){1920 * 0.15f, 248}, 22, 1, GRAY);
    DrawTextEx(font, "Абдульманов Данияр Айдарович", (Vector2){1920 * 0.15f + 200, 248}, 22, 1, WHITE);
    DrawTextEx(font, "Сергунин Роман Дмитриевич", (Vector2){1920 * 0.15f + 200, 284}, 22, 1, WHITE);

    DrawTextEx(font, "Группа:", (Vector2){1920 * 0.15f, 338}, 22, 1, GRAY);
    DrawTextEx(font, "5131001/50601", (Vector2){1920 * 0.15f + 200, 338}, 22, 1, WHITE);

    DrawTextEx(font, "Год:", (Vector2){1920 * 0.15f, 392}, 22, 1, GRAY);
    DrawTextEx(font, "2026", (Vector2){1920 * 0.15f + 200, 392}, 22, 1, WHITE);

    DrawTextEx(font, "ВУЗ:", (Vector2){1920 * 0.15f, 446}, 22, 1, GRAY);
    DrawTextEx(font, "СПбГУ им. Петра Великого", (Vector2){1920 * 0.15f + 200, 446}, 22, 1, WHITE);

    DrawTextEx(font, "Институт:", (Vector2){1920 * 0.15f, 500}, 22, 1, GRAY);
    DrawTextEx(font, "Институт компьютерных наук и кибербезопасности", (Vector2){1920 * 0.15f + 200, 500}, 22, 1, WHITE);

    DrawTextEx(font, "Кафедра:", (Vector2){1920 * 0.15f, 554}, 22, 1, GRAY);
    DrawTextEx(font, "Высшая Школа Кибербезопасности", (Vector2){1920 * 0.15f + 200, 554}, 22, 1, WHITE);
}

static void DrawHelp(Font font, Screen *screen)
{
    Rectangle backBtn = {20, 20, 170, 44};
    Vector2 m = GetMousePosition();

    DrawBackButton(font, m, backBtn, "< МЕНЮ", screen, MENU);

    Vector2 ts = MeasureTextEx(font, "ПОМОЩЬ", 52, 1);
    DrawTextEx(font, "ПОМОЩЬ", (Vector2){(1920 - ts.x) * 0.5f, 76}, 52, 1, WHITE);
    DrawRectangle(1920 / 2 - 320, 148, 640, 2, (Color){40,190,210,100});

    DrawTextEx(font, "ПРАВИЛА ИГРЫ", (Vector2){1920 * 0.12f, 180}, 28, 1, (Color){40,190,210,255});
    DrawRectangle(1920 * 0.12f, 216, 700, 1, (Color){60,60,90,255});

    DrawTextEx(font, "Цель - провести своего персонажа от стартовой точки до выхода из лабиринта.",
                (Vector2){1920 * 0.12f, 230}, 22, 1, WHITE);
    DrawTextEx(font, "Лабиринт генерируется случайно, но всегда имеет единственное решение.",
                (Vector2){1920 * 0.12f, 268}, 22, 1, WHITE);
    DrawTextEx(font, "Размер лабиринта задаётся вручную: ширина и высота от 3 до 100 клеток.",
                (Vector2){1920 * 0.12f, 306}, 22, 1, (Color){255,220,80,255});
    DrawTextEx(font, "Значения больше 100 автоматически обрезаются до 0.",
                (Vector2){1920 * 0.12f, 344}, 22, 1, (Color){180,180,180,255});

    DrawTextEx(font, "УПРАВЛЕНИЕ", (Vector2){1920 * 0.12f, 410}, 28, 1, (Color){40,190,210,255});
    DrawRectangle(1920 * 0.12f, 446, 700, 1, (Color){60,60,90,255});

    float cx = 1920 * 0.12f;
    float cy = 460;
    float rowH = 46;

    const char *keys[] = {
        "W / Стрелка вверх",
        "S / Стрелка вниз",
        "A / Стрелка влево",
        "D / Стрелка вправо",
        "Esc",
        "R"
    };

    const char *acts[] = {
        "Движение вверх",
        "Движение вниз",
        "Движение влево",
        "Движение вправо",
        "Пауза / Меню",
        "Рестарт уровня"
    };

    for (int i = 0; i < 6; i++)
    {
        Color rowColor = (i % 2 == 0) ? (Color){20,20,36,200} : (Color){28,28,52,200};
        DrawRectangle((int)cx - 10, (int)(cy + i * rowH) - 4, 860, (int)rowH - 2, rowColor);
        DrawTextEx(font, keys[i], (Vector2){cx, cy + i * rowH}, 22, 1, (Color){100,220,255,255});
        DrawTextEx(font, "-", (Vector2){cx + 340, cy + i * rowH}, 22, 1, GRAY);
        DrawTextEx(font, acts[i], (Vector2){cx + 380, cy + i * rowH}, 22, 1, WHITE);
    }
}

static void DrawBotVsBot(Font font, Screen *screen)
{
    Rectangle backBtn = {20, 20, 170, 44};
    Vector2 m = GetMousePosition();

    DrawBackButton(font, m, backBtn, "< НАЗАД", screen, MODE_SELECT);

    Vector2 ts = MeasureTextEx(font, "БОТ ПРОТИВ БОТА", 52, 1);
    DrawTextEx(font, "БОТ ПРОТИВ БОТА", (Vector2){(1920 - ts.x) * 0.5f, 200}, 52, 1, WHITE);

    Vector2 sub = MeasureTextEx(font, "Режим в разработке...", 32, 1);
    DrawTextEx(font, "Режим в разработке...", (Vector2){(1920 - sub.x) * 0.5f, 540}, 32, 1, (Color){120,120,140,255});
}

static void DrawKeyInput(Font font, Screen *screen)
{
    Rectangle backBtn = {20, 20, 170, 44};
    Vector2 m = GetMousePosition();

    DrawBackButton(font, m, backBtn, "< НАЗАД", screen, MODE_SELECT);

    Vector2 ts = MeasureTextEx(font, "ВВОД КЛЮЧА", 52, 1);
    DrawTextEx(font, "ВВОД КЛЮЧА", (Vector2){(1920 - ts.x) * 0.5f, 200}, 52, 1, WHITE);

    Vector2 sub = MeasureTextEx(font, "Функция в разработке...", 32, 1);
    DrawTextEx(font, "Функция в разработке...", (Vector2){(1920 - sub.x) * 0.5f, 540}, 32, 1, (Color){120,120,140,255});
}

void RunMazeGenerator(void)
{
    InitWindow(1920, 1080, "Maze Generator");
    SetTargetFPS(144);

    int cp[200], n = 0;
    for (int i = 32; i <= 126; i++) cp[n++] = i;
    for (int i = 1040; i <= 1103; i++) cp[n++] = i;
    cp[n++] = 1025;
    cp[n++] = 1105;

    Font font = LoadFontEx(FONT_PATH, 96, cp, n);
    SetTextureFilter(font.texture, TEXTURE_FILTER_BILINEAR);

    Screen screen = MENU;

    int width = 0, height = 0;
    int genW = 0, genH = 0;
    int active = -1;

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground((Color){12, 12, 22, 255});

        if (screen == MENU)
            DrawMenu(font, &screen);
        else if (screen == MODE_SELECT)
            DrawModeSelect(font, &screen);
        else if (screen == GAME)
            DrawGame(font, &screen, &width, &height, &genW, &genH, &active);
        else if (screen == ABOUT)
            DrawAbout(font, &screen);
        else if (screen == HELP)
            DrawHelp(font, &screen);
        else if (screen == BOT_VS_BOT)
            DrawBotVsBot(font, &screen);
        else if (screen == KEY_INPUT)
            DrawKeyInput(font, &screen);

        EndDrawing();
    }

    UnloadFont(font);
    CloseWindow();
}