#include "graphic.h"
#include "labyrinth.h"
#include "error.h"
#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define FONT_PATH "DejaVuSansBold.ttf"

typedef enum {
    MENU,
    LOGIN,
    MODE_SELECT,
    GAME,
    GENERATING,
    PLAYING,
    ABOUT,
    HELP,
    BOT_VS_BOT,
    KEY_INPUT
} Screen;

// логин пользователя
static char login_name[33] = {0};
static int login_len = 0;
static int login_error  = 0; // 1 = пустой логин

// состояние прогресс-бара генерации
static float gen_timer = 0.0f;
static int gen_ok = 0;
#define GEN_DURATION 1.8f

//назад
static void DrawBackButton(Font font, Vector2 m, Rectangle backBtn,
                           const char *label, Screen *screen, Screen target)
{
    int hot = CheckCollisionPointRec(m, backBtn);
    DrawRectangleRec(backBtn, hot ? (Color){55,55,68,255} : (Color){35,35,48,255});
    DrawRectangleLinesEx(backBtn, 2, GRAY);
    Vector2 bt = MeasureTextEx(font, label, 18, 1);
    DrawTextEx(font, label,
               (Vector2){backBtn.x + (backBtn.width  - bt.x) * 0.5f,
                         backBtn.y + (backBtn.height - bt.y) * 0.5f},
               18, 1, LIGHTGRAY);
    if (hot && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        *screen = target;
}


//меню
static void DrawMenu(Font font, Screen *screen)
{
    Vector2 ts = MeasureTextEx(font, "MAZE GENERATOR", 88, 1);
    DrawTextEx(font, "MAZE GENERATOR", (Vector2){(1920 - ts.x) * 0.5f, 110}, 88, 1, WHITE);

    Rectangle btnStart = {(1920 - 320) * 0.5f, 400, 320, 64};
    Rectangle btnHelp  = {(1920 - 320) * 0.5f, 490, 320, 64};
    Rectangle btnAbout = {(1920 - 320) * 0.5f, 580, 320, 64};
    Rectangle btnExit  = {(1920 - 320) * 0.5f, 670, 320, 64};

    Vector2 m = GetMousePosition();

    // играть потом логин
    int hotStart = CheckCollisionPointRec(m, btnStart);
    DrawRectangleRec(btnStart, hotStart ? (Color){15,55,140,255} : (Color){10,40,100,255});
    DrawRectangleLinesEx(btnStart, 2, hotStart ? WHITE : (Color){60,120,255,255});
    Vector2 st = MeasureTextEx(font, "ИГРАТЬ", 28, 1);
    DrawTextEx(font, "ИГРАТЬ",
               (Vector2){btnStart.x + (btnStart.width  - st.x) * 0.5f,
                         btnStart.y + (btnStart.height - st.y) * 0.5f},
               28, 1, WHITE);
    if (hotStart && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        login_name[0] = '\0';
        login_len     = 0;
        login_error   = 0;
        *screen = LOGIN;
    }

    //помощь
    int hotHelp = CheckCollisionPointRec(m, btnHelp);
    DrawRectangleRec(btnHelp, hotHelp ? (Color){20,80,90,255} : (Color){14,58,68,255});
    DrawRectangleLinesEx(btnHelp, 2, hotHelp ? WHITE : (Color){40,190,210,255});
    Vector2 ht = MeasureTextEx(font, "ПОМОЩЬ", 28, 1);
    DrawTextEx(font, "ПОМОЩЬ",
               (Vector2){btnHelp.x + (btnHelp.width  - ht.x) * 0.5f,
                         btnHelp.y + (btnHelp.height - ht.y) * 0.5f},
               28, 1, WHITE);
    if (hotHelp && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) *screen = HELP;

    //справка
    int hotAbout = CheckCollisionPointRec(m, btnAbout);
    DrawRectangleRec(btnAbout, hotAbout ? (Color){80,35,110,255} : (Color){60,25,85,255});
    DrawRectangleLinesEx(btnAbout, 2, hotAbout ? WHITE : (Color){170,80,255,255});
    Vector2 at = MeasureTextEx(font, "СПРАВКА", 28, 1);
    DrawTextEx(font, "СПРАВКА",
               (Vector2){btnAbout.x + (btnAbout.width  - at.x) * 0.5f,
                         btnAbout.y + (btnAbout.height - at.y) * 0.5f},
               28, 1, WHITE);
    if (hotAbout && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) *screen = ABOUT;

    //выход
    int hotExit = CheckCollisionPointRec(m, btnExit);
    DrawRectangleRec(btnExit, hotExit ? (Color){120,20,20,255} : (Color){90,15,15,255});
    DrawRectangleLinesEx(btnExit, 2, hotExit ? WHITE : (Color){220,60,60,255});
    Vector2 et = MeasureTextEx(font, "ВЫХОД", 28, 1);
    DrawTextEx(font, "ВЫХОД",
               (Vector2){btnExit.x + (btnExit.width  - et.x) * 0.5f,
                         btnExit.y + (btnExit.height - et.y) * 0.5f},
               28, 1, WHITE);
    if (hotExit && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        log_info("Application closed by user via menu");
        log_close();
        CloseWindow();
    }
}


//логин
static void DrawLogin(Font font, Screen *screen)
{
    Rectangle backBtn    = {20, 20, 170, 44};
    Rectangle inputBox   = {(1920 - 520) * 0.5f, 470, 520, 62};
    Rectangle confirmBtn = {(1920 - 300) * 0.5f, 582, 300, 60};

    Vector2 m = GetMousePosition();

    DrawBackButton(font, m, backBtn, "< МЕНЮ", screen, MENU);

    // заголовок
    Vector2 ts = MeasureTextEx(font, "ВХОД В ИГРУ", 52, 1);
    DrawTextEx(font, "ВХОД В ИГРУ",
               (Vector2){(1920 - ts.x) * 0.5f, 200}, 52, 1, WHITE);
    DrawRectangle(1920/2 - 280, 268, 560, 2, (Color){60,120,255,80});

    Vector2 ss = MeasureTextEx(font, "Введите ваш уникальный логин", 26, 1);
    DrawTextEx(font, "Введите ваш уникальный логин",
               (Vector2){(1920 - ss.x) * 0.5f, 390}, 26, 1, LIGHTGRAY);

    // поле ввода
    DrawRectangleRec(inputBox, (Color){15, 15, 32, 255});
    DrawRectangleLinesEx(inputBox, 2, (Color){60, 120, 255, 255});

    // мигающий курсор
    static int blink = 0;
    blink++;
    char displayBuf[36];
    if ((blink / 36) % 2 == 0)
        snprintf(displayBuf, sizeof(displayBuf), "%s|", login_name);
    else
        snprintf(displayBuf, sizeof(displayBuf), "%s ", login_name);

    Vector2 lt = MeasureTextEx(font, displayBuf, 28, 1);
    float tx = inputBox.x + (inputBox.width - lt.x) * 0.5f;
    if (tx < inputBox.x + 10) tx = inputBox.x + 10;
    DrawTextEx(font, displayBuf,
               (Vector2){tx, inputBox.y + (inputBox.height - lt.y) * 0.5f},
               28, 1, (Color){80, 200, 120, 255});

    // подсказка
    DrawTextEx(font, "Только латинские буквы и цифры, до 32 символов",
               (Vector2){inputBox.x, inputBox.y + inputBox.height + 10},
               16, 1, (Color){80, 80, 110, 255});

    // ввод с клавиатуры (ASCII 33-126, без пробела)
    int key = GetCharPressed();
    while (key > 0)
    {
        if (key > 32 && key <= 126 && login_len < 32)
        {
            login_name[login_len++] = (char)key;
            login_name[login_len]   = '\0';
            login_error = 0;
        }
        key = GetCharPressed();
    }
    if (IsKeyPressed(KEY_BACKSPACE) && login_len > 0)
    {
        login_name[--login_len] = '\0';
        login_error = 0;
    }

    //продолжить
    int hotConf = CheckCollisionPointRec(m, confirmBtn);
    DrawRectangleRec(confirmBtn, hotConf ? (Color){15,55,140,255} : (Color){10,40,100,255});
    DrawRectangleLinesEx(confirmBtn, 2, hotConf ? WHITE : (Color){60,120,255,255});
    Vector2 ct = MeasureTextEx(font, "ПРОДОЛЖИТЬ", 24, 1);
    DrawTextEx(font, "ПРОДОЛЖИТЬ",
               (Vector2){confirmBtn.x + (confirmBtn.width  - ct.x) * 0.5f,
                         confirmBtn.y + (confirmBtn.height - ct.y) * 0.5f},
               24, 1, WHITE);

    int doConfirm = (hotConf && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                 || IsKeyPressed(KEY_ENTER);
    if (doConfirm)
    {
        if (login_len > 0)
        {
            char logbuf[80];
            snprintf(logbuf, sizeof(logbuf), "User logged in: %s", login_name);
            log_info(logbuf);
            login_error = 0;
            *screen = MODE_SELECT;
        }
        else
        {
            login_error = 1;
            log_error("Login attempt with empty username");
        }
    }

    // ошибка пустого логина
    if (login_error)
    {
        Vector2 errt = MeasureTextEx(font, "Логин не может быть пустым!", 22, 1);
        DrawTextEx(font, "Логин не может быть пустым!",
                   (Vector2){(1920 - errt.x) * 0.5f, confirmBtn.y + confirmBtn.height + 18},
                   22, 1, RED);
    }

    // live-превью логина
    if (login_len > 0 && !login_error)
    {
        char greet[64];
        snprintf(greet, sizeof(greet), "Привет, %s!", login_name);
        Vector2 gt = MeasureTextEx(font, greet, 20, 1);
        DrawTextEx(font, greet,
                   (Vector2){(1920 - gt.x) * 0.5f, confirmBtn.y + confirmBtn.height + 18},
                   20, 1, (Color){60, 180, 100, 200});
    }
}


//выбор режима
static void DrawModeSelect(Font font, Screen *screen)
{
    Rectangle backBtn   = {20, 20, 170, 44};
    Rectangle btnSolo   = {(1920 - 400) * 0.5f, 380, 400, 70};
    Rectangle btnBotBot = {(1920 - 400) * 0.5f, 480, 400, 70};
    Rectangle btnKey    = {(1920 - 400) * 0.5f, 580, 400, 70};

    Vector2 m = GetMousePosition();

    Vector2 title = MeasureTextEx(font, "ВЫБОР РЕЖИМА", 52, 1);
    DrawTextEx(font, "ВЫБОР РЕЖИМА", (Vector2){(1920 - title.x) * 0.5f, 200}, 52, 1, WHITE);
    DrawRectangle(1920/2 - 300, 270, 600, 2, (Color){60,60,90,255});

    // имя игрока в правом верхнем углу
    char nameLabel[48];
    snprintf(nameLabel, sizeof(nameLabel), "Игрок: %s", login_name);
    DrawTextEx(font, nameLabel, (Vector2){1920 - 400.0f, 24.0f}, 20, 1, (Color){80,180,120,200});

    DrawBackButton(font, m, backBtn, "< МЕНЮ", screen, MENU);

    int hotSolo = CheckCollisionPointRec(m, btnSolo);
    DrawRectangleRec(btnSolo, hotSolo ? (Color){15,55,140,255} : (Color){10,40,100,255});
    DrawRectangleLinesEx(btnSolo, 2, hotSolo ? WHITE : (Color){60,120,255,255});
    Vector2 s1 = MeasureTextEx(font, "В ОДИНОЧКУ", 28, 1);
    DrawTextEx(font, "В ОДИНОЧКУ",
               (Vector2){btnSolo.x + (btnSolo.width  - s1.x) * 0.5f,
                         btnSolo.y + (btnSolo.height - s1.y) * 0.5f},
               28, 1, WHITE);
    if (hotSolo && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) *screen = GAME;

    int hotBotBot = CheckCollisionPointRec(m, btnBotBot);
    DrawRectangleRec(btnBotBot, hotBotBot ? (Color){80,35,110,255} : (Color){60,25,85,255});
    DrawRectangleLinesEx(btnBotBot, 2, hotBotBot ? WHITE : (Color){170,80,255,255});
    Vector2 s2 = MeasureTextEx(font, "БОТ ПРОТИВ БОТА", 28, 1);
    DrawTextEx(font, "БОТ ПРОТИВ БОТА",
               (Vector2){btnBotBot.x + (btnBotBot.width  - s2.x) * 0.5f,
                         btnBotBot.y + (btnBotBot.height - s2.y) * 0.5f},
               28, 1, WHITE);
    if (hotBotBot && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) *screen = BOT_VS_BOT;

    int hotKey = CheckCollisionPointRec(m, btnKey);
    DrawRectangleRec(btnKey, hotKey ? (Color){20,80,90,255} : (Color){14,58,68,255});
    DrawRectangleLinesEx(btnKey, 2, hotKey ? WHITE : (Color){40,190,210,255});
    Vector2 s3 = MeasureTextEx(font, "ВВЕСТИ КЛЮЧ", 28, 1);
    DrawTextEx(font, "ВВЕСТИ КЛЮЧ",
               (Vector2){btnKey.x + (btnKey.width  - s3.x) * 0.5f,
                         btnKey.y + (btnKey.height - s3.y) * 0.5f},
               28, 1, WHITE);
    if (hotKey && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) *screen = KEY_INPUT;
}


//параметры лабиринта от польователя
static void DrawGame(Font font, Screen *screen,
                     int *inputW, int *inputH,
                     int *active, int *px, int *py, int *errorFlag)
{
    Rectangle backBtn   = {20,   20,  170, 44};
    Rectangle widthBox  = {384,  238, 220, 52};
    Rectangle heightBox = {1152, 238, 220, 52};
    Rectangle genBtn    = {810,  410, 300, 62};

    Vector2 m = GetMousePosition();

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        if(CheckCollisionPointRec(m, widthBox))  { *active = 0; }
        else if (CheckCollisionPointRec(m, heightBox)) { *active = 1; }
        else if (CheckCollisionPointRec(m, genBtn))
        {
            if (*inputW >= 3 && *inputH >= 3)
            {
                *errorFlag = 0;
                width  = (size_l)(*inputW);
                height = (size_l)(*inputH);
                srand((unsigned int)time(NULL));

                // синхронная генерация — после неё запускается анимация прогресс-бара
                int ok = 0;
                for (int attempt = 0; attempt < 200 && !ok; attempt++)
                {
                    free_lab();
                    if (generate_lab() == 1 && find_way(1, 1))
                        ok = 1;
                }

                if (ok)
                {
                    char logbuf[80];
                    snprintf(logbuf, sizeof(logbuf),
                             "Labyrinth %dx%d generated, user=%s", *inputW, *inputH, login_name);
                    log_info(logbuf);
                    gen_ok    = 1;
                    gen_timer = 0.0f;
                    *px = 1; *py = 1;
                    *screen = GENERATING;
                }
                else
                {
                    log_error("Failed to generate labyrinth after 200 attempts");
                    *errorFlag = 2;
                }
            }
            else
            {
                log_error("DrawGame: labyrinth size < 3 requested");
                *errorFlag = 1;
            }
        }
        else if (CheckCollisionPointRec(m, backBtn)) { *screen = MODE_SELECT; }
        else { *active = -1; }
    }

    // ввод цифр
    int key = GetCharPressed();
    while (key > 0)
    {
        if (key >= '0' && key <= '9')
        {
            if (*active == 0) { int v = (*inputW)*10+(key-'0'); *inputW = (v>100)?0:v; }
            if (*active == 1) { int v = (*inputH)*10+(key-'0'); *inputH = (v>100)?0:v; }
        }
        key = GetCharPressed();
    }
    if (IsKeyPressed(KEY_BACKSPACE))
    {
        if (*active == 0) *inputW /= 10;
        if (*active == 1) *inputH /= 10;
    }

    DrawBackButton(font, m, backBtn, "< НАЗАД", screen, MODE_SELECT);

    Vector2 titleV = MeasureTextEx(font, "НАСТРОЙКА ЛАБИРИНТА", 42, 1);
    DrawTextEx(font, "НАСТРОЙКА ЛАБИРИНТА",
               (Vector2){(1920 - titleV.x) * 0.5f, 86}, 42, 1, WHITE);

    DrawTextEx(font, "ШИРИНА", (Vector2){widthBox.x,widthBox.y  - 26}, 18, 1, LIGHTGRAY);
    DrawTextEx(font, "ВЫСОТА",(Vector2){heightBox.x, heightBox.y - 26}, 18, 1, LIGHTGRAY);
    DrawTextEx(font, "(макс. 100)",(Vector2){widthBox.x, widthBox.y  + 58}, 16, 1, (Color){100,100,130,255});
    DrawTextEx(font, "(макс. 100)",(Vector2){heightBox.x,heightBox.y + 58}, 16, 1, (Color){100,100,130,255});

    DrawRectangleLinesEx(widthBox,  2, *active == 0 ? BLUE : GRAY);
    DrawRectangleLinesEx(heightBox, 2, *active == 1 ? BLUE : GRAY);
    DrawTextEx(font, TextFormat("%d", *inputW),
               (Vector2){widthBox.x  + 12, widthBox.y  + 13}, 26, 1, GREEN);
    DrawTextEx(font, TextFormat("%d", *inputH),
               (Vector2){heightBox.x + 12, heightBox.y + 13}, 26, 1, GREEN);

    int hotGen = CheckCollisionPointRec(m, genBtn);
    DrawRectangleRec(genBtn, hotGen ? (Color){15,80,28,255} : (Color){8,55,18,255});
    DrawRectangleLinesEx(genBtn, 2, hotGen ? WHITE : (Color){50,200,80,255});
    Vector2 gt = MeasureTextEx(font, "СГЕНЕРИРОВАТЬ", 24, 1);
    DrawTextEx(font, "СГЕНЕРИРОВАТЬ",
               (Vector2){genBtn.x + (genBtn.width  - gt.x) * 0.5f,
                         genBtn.y + (genBtn.height - gt.y) * 0.5f},
               24, 1, WHITE);

    if (*errorFlag == 1)
    {
        Vector2 errt = MeasureTextEx(font, "Размер должен быть не менее 3!", 22, 1);
        DrawTextEx(font, "Размер должен быть не менее 3!",
                   (Vector2){(1920 - errt.x) * 0.5f, 500}, 22, 1, RED);
    }
    else if (*errorFlag == 2)
    {
        Vector2 errt = MeasureTextEx(font, "Не удалось создать лабиринт. Попробуйте другой размер.", 22, 1);
        DrawTextEx(font, "Не удалось создать лабиринт. Попробуйте другой размер.",
                   (Vector2){(1920 - errt.x) * 0.5f, 500}, 22, 1, RED);
    }
}


//прогресс бар
static void DrawGenerating(Font font, Screen *screen, int *px, int *py)
{
    gen_timer += GetFrameTime();

    float progress = gen_timer / GEN_DURATION;
    if (progress > 1.0f) progress = 1.0f;

    // заголовок
    Vector2 ts = MeasureTextEx(font, "ГЕНЕРАЦИЯ ЛАБИРИНТА", 48, 1);
    DrawTextEx(font, "ГЕНЕРАЦИЯ ЛАБИРИНТА",
               (Vector2){(1920 - ts.x) * 0.5f, 340}, 48, 1, WHITE);

    // имя игрока
    char nameLabel[48];
    snprintf(nameLabel, sizeof(nameLabel), "Игрок: %s", login_name);
    Vector2 nl = MeasureTextEx(font, nameLabel, 22, 1);
    DrawTextEx(font, nameLabel,
               (Vector2){(1920 - nl.x) * 0.5f, 412}, 22, 1, (Color){80,180,120,210});

    // прогресс-бар
    float barW = 720.0f, barH = 46.0f;
    float barX = (1920 - barW) * 0.5f;
    float barY = 490.0f;

    //фон
    DrawRectangle((int)barX, (int)barY, (int)barW, (int)barH, (Color){22, 22, 42, 255});

    // заполненная часть с градиентом
    int fillW = (int)(barW * progress);
    if (fillW > 0)
        DrawRectangleGradientH((int)barX, (int)barY, fillW, (int)barH,
                               (Color){0, 140, 60, 255}, (Color){0, 220, 100, 255});

    // блик сверху
    if (fillW > 4)
        DrawRectangle((int)barX + 2, (int)barY + 2, fillW - 4, 7,
                      (Color){180, 255, 200, 35});

    //рамка
    DrawRectangleLinesEx((Rectangle){barX, barY, barW, barH}, 2, (Color){55, 55, 88, 255});

    //процент загрузки
    char pctText[8];
    snprintf(pctText, sizeof(pctText), "%d%%", (int)(progress * 100.0f));
    Vector2 pt = MeasureTextEx(font, pctText, 22, 1);
    DrawTextEx(font, pctText,
               (Vector2){barX + (barW - pt.x) * 0.5f, barY + (barH - pt.y) * 0.5f},
               22, 1, WHITE);

    // анимированная подпись
    const char *dots[] = {
        "Пожалуйста, подождите.",
        "Пожалуйста, подождите..",
        "Пожалуйста, подождите..."
    };
    int dotIdx = (int)(gen_timer * 2.5f) % 3;
    Vector2 sub = MeasureTextEx(font, dots[dotIdx], 22, 1);
    DrawTextEx(font, dots[dotIdx],
               (Vector2){(1920 - sub.x) * 0.5f, barY + barH + 26},
               22, 1, (Color){80, 80, 120, 255});

    // переход по окончании анимации
    if (gen_timer >= GEN_DURATION)
    {
        if (gen_ok)
        {
            reset_path_marks();
            *px = 1; *py = 1;
            log_info("Entering PLAYING screen");
            *screen = PLAYING;
        }
        else
        {
            log_error("DrawGenerating: gen_ok=0 on finish, returning to GAME");
            *screen = GAME;
        }
    }
}


//процесс игры
static void DrawPlaying(Font font, Screen *screen, int *px, int *py)
{
    if (map == NULL) { *screen = GAME; return; }

    int won = (*px == (int)x_end && *py == (int)y_end);

    if (!won)
    {
        int nx = *px, ny = *py;
        if      (IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP))   ny--;
        else if (IsKeyPressed(KEY_S) || IsKeyPressed(KEY_DOWN))  ny++;
        else if (IsKeyPressed(KEY_A) || IsKeyPressed(KEY_LEFT))  nx--;
        else if (IsKeyPressed(KEY_D) || IsKeyPressed(KEY_RIGHT)) nx++;

        if ((nx != *px || ny != *py) &&
            nx >= 0 && nx < (int)width &&
            ny >= 0 && ny < (int)height &&
            map[ny][nx] != wall)
        { *px = nx; *py = ny; }

        if (IsKeyPressed(KEY_R)) { *px = 1; *py = 1; }
        if (IsKeyPressed(KEY_ESCAPE)) { *screen = GAME; return; }
    }

    // размер клетки
    float cellW = 1880.0f / (float)width;
    float cellH = 1020.0f / (float)height;
    float cellSize = (cellW < cellH) ? cellW : cellH;
    if (cellSize > 40.0f) cellSize = 40.0f;
    if (cellSize < 1.0f)  cellSize = 1.0f;

    float mazePixW = cellSize * (float)width;
    float mazePixH = cellSize * (float)height;
    float offX = (1920.0f - mazePixW) * 0.5f;
    float offY = (1080.0f - mazePixH) * 0.5f;

    //сетка
    for (int r = 0; r < (int)height; r++)
        for (int c = 0; c < (int)width; c++)
        {
            Color col = (map[r][c] == wall) ? (Color){38,38,62,255} : (Color){8,8,20,255};
            DrawRectangleV((Vector2){offX + c*cellSize, offY + r*cellSize},
                           (Vector2){cellSize, cellSize}, col);
        }

    //старт
    DrawRectangleV((Vector2){offX + 1*cellSize, offY + 1*cellSize},
                   (Vector2){cellSize, cellSize}, (Color){20,80,180,120});

    //выход
    DrawRectangleV((Vector2){offX + (int)x_end*cellSize, offY + (int)y_end*cellSize},
                   (Vector2){cellSize, cellSize}, (Color){0,200,80,255});
    if (cellSize >= 10.0f)
    {
        Vector2 ev = MeasureTextEx(font, "В", cellSize*0.55f, 1);
        DrawTextEx(font, "В",
                   (Vector2){offX+(int)x_end*cellSize+(cellSize-ev.x)*0.5f,
                             offY+(int)y_end*cellSize+(cellSize-ev.y)*0.5f},
                   cellSize*0.55f, 1, (Color){0,255,100,255});
    }

    // игрок
    float margin = cellSize * 0.12f;
    float ps = cellSize - 2.0f*margin;
    DrawRectangleV((Vector2){offX + *px*cellSize + margin, offY + *py*cellSize + margin},
                   (Vector2){ps, ps}, (Color){80,140,255,255});

    // подсказки
    DrawTextEx(font, "WASD / стрелки - движение     R - рестарт     ESC - настройки",
               (Vector2){20.0f, 1056.0f}, 16, 1, (Color){70,70,100,255});

    // имя игрока
    char nameLabel[48];
    snprintf(nameLabel, sizeof(nameLabel), "Игрок: %s", login_name);
    DrawTextEx(font, nameLabel, (Vector2){1920 - 300.0f, 12.0f}, 18, 1, (Color){80,180,120,180});

    // кнопка назад
    {
        Rectangle backBtn = {20, 20, 200, 44};
        Vector2 bm = GetMousePosition();
        DrawBackButton(font, bm, backBtn, "< НАСТРОЙКИ", screen, GAME);
    }

    // победа
    if (won)
    {
        static int winLogged = 0;
        if (!winLogged)
        {
            char logbuf[80];
            snprintf(logbuf, sizeof(logbuf), "User %s reached the exit!", login_name);
            log_info(logbuf);
            winLogged = 1;
        }

        DrawRectangle(0, 0, 1920, 1080, (Color){0,0,0,170});

        const char *msgWin = "ВЫХОД НАЙДЕН!";
        Vector2 wt = MeasureTextEx(font, msgWin, 80, 1);
        DrawTextEx(font, msgWin,
                   (Vector2){(1920.0f-wt.x)*0.5f, 420.0f}, 80, 1, (Color){0,220,80,255});

        const char *msgSub = "R - новый лабиринт     ESC - к настройкам";
        Vector2 st = MeasureTextEx(font, msgSub, 30, 1);
        DrawTextEx(font, msgSub,(Vector2){(1920.0f-st.x)*0.5f, 540.0f}, 30, 1, LIGHTGRAY);

        if (IsKeyPressed(KEY_R))
        {
            winLogged = 0;
            srand((unsigned int)time(NULL));
            int ok = 0;
            for (int attempt = 0; attempt < 200 && !ok; attempt++)
            {
                free_lab();
                if (generate_lab() == 1 && find_way(1, 1)) ok = 1;
            }
            if (ok)
            {
                log_info("New labyrinth generated from win screen");
                gen_ok = 1; gen_timer = 0.0f;
                *screen = GENERATING;
            }
            else
            {
                log_error("Failed to generate new labyrinth from win screen");
                *screen = GAME;
            }
        }
        if (IsKeyPressed(KEY_ESCAPE)) { winLogged = 0; *screen = GAME; }
    }
}


//справка
static void DrawAbout(Font font, Screen *screen)
{
    Rectangle backBtn = {20, 20, 170, 44};
    Vector2 m = GetMousePosition();
    DrawBackButton(font, m, backBtn, "< МЕНЮ", screen, MENU);

    Vector2 ts = MeasureTextEx(font, "СПРАВКА", 52, 1);
    DrawTextEx(font, "СПРАВКА", (Vector2){(1920-ts.x)*0.5f, 76}, 52, 1, WHITE);
    DrawRectangle(1920/2-280, 194, 560, 2, (Color){80,80,110,255});

    DrawTextEx(font, "Авторы:", (Vector2){1920*0.15f, 248}, 22, 1, GRAY);
    DrawTextEx(font, "Абдульманов Данияр Айдарович", (Vector2){1920*0.15f+200, 248}, 22, 1, WHITE);
    DrawTextEx(font, "Сергунин Роман Дмитриевич", (Vector2){1920*0.15f+200, 284}, 22, 1, WHITE);
    DrawTextEx(font, "Группа:",(Vector2){1920*0.15f, 338}, 22, 1, GRAY);
    DrawTextEx(font, "5131001/50601", (Vector2){1920*0.15f+200, 338}, 22, 1, WHITE);
    DrawTextEx(font, "Год:", (Vector2){1920*0.15f, 392}, 22, 1, GRAY);
    DrawTextEx(font, "2026", (Vector2){1920*0.15f+200, 392}, 22, 1, WHITE);
    DrawTextEx(font, "ВУЗ:",(Vector2){1920*0.15f, 446}, 22, 1, GRAY);
    DrawTextEx(font, "СПбГУ им. Петра Великого", (Vector2){1920*0.15f+200, 446}, 22, 1, WHITE);
    DrawTextEx(font, "Институт:", (Vector2){1920*0.15f, 500}, 22, 1, GRAY);
    DrawTextEx(font, "Институт компьютерных наук и кибербезопасности",
               (Vector2){1920*0.15f+200, 500}, 22, 1, WHITE);
    DrawTextEx(font, "Кафедра:",(Vector2){1920*0.15f, 554}, 22, 1, GRAY);
    DrawTextEx(font, "Высшая Школа Кибербезопасности",
               (Vector2){1920*0.15f+200, 554}, 22, 1, WHITE);
}


//помощь
static void DrawHelp(Font font, Screen *screen)
{
    Rectangle backBtn = {20, 20, 170, 44};
    Vector2 m = GetMousePosition();
    DrawBackButton(font, m, backBtn, "< МЕНЮ", screen, MENU);

    Vector2 ts = MeasureTextEx(font, "ПОМОЩЬ", 52, 1);
    DrawTextEx(font, "ПОМОЩЬ", (Vector2){(1920-ts.x)*0.5f, 76}, 52, 1, WHITE);
    DrawRectangle(1920/2-320, 148, 640, 2, (Color){40,190,210,100});

    DrawTextEx(font, "ПРАВИЛА ИГРЫ", (Vector2){1920*0.12f, 180}, 28, 1, (Color){40,190,210,255});
    DrawRectangle(1920*0.12f, 216, 700, 1, (Color){60,60,90,255});
    DrawTextEx(font, "Цель - провести своего персонажа от стартовой точки до выхода из лабиринта.",
               (Vector2){1920*0.12f, 230}, 22, 1, WHITE);
    DrawTextEx(font, "Лабиринт генерируется случайно, но всегда имеет единственное решение.",
               (Vector2){1920*0.12f, 268}, 22, 1, WHITE);
    DrawTextEx(font, "Размер лабиринта задаётся вручную: ширина и высота от 3 до 100 клеток.",
               (Vector2){1920*0.12f, 306}, 22, 1, (Color){255,220,80,255});
    DrawTextEx(font, "Значения больше 100 автоматически обрезаются до 0.",
               (Vector2){1920*0.12f, 344}, 22, 1, (Color){180,180,180,255});

    DrawTextEx(font, "УПРАВЛЕНИЕ", (Vector2){1920*0.12f, 410}, 28, 1, (Color){40,190,210,255});
    DrawRectangle(1920*0.12f, 446, 700, 1, (Color){60,60,90,255});

    float cx = 1920*0.12f, cy = 460, rowH = 46;
    const char *keys[] = {
        "W / Стрелка вверх","S / Стрелка вниз",
        "A / Стрелка влево","D / Стрелка вправо",
        "R","ESC"
    };
    const char *acts[] = {
        "Движение вверх","Движение вниз",
        "Движение влево","Движение вправо",
        "Рестарт (вернуться на старт)",
        "Пауза / вернуться к настройкам"
    };
    for (int i = 0; i < 6; i++)
    {
        Color rowColor = (i%2==0) ? (Color){20,20,36,200} : (Color){28,28,52,200};
        DrawRectangle((int)cx-10, (int)(cy+i*rowH)-4, 900, (int)rowH-2, rowColor);
        DrawTextEx(font, keys[i], (Vector2){cx, cy+i*rowH}, 22, 1, (Color){100,220,255,255});
        DrawTextEx(font, "-",(Vector2){cx+340, cy+i*rowH}, 22, 1, GRAY);
        DrawTextEx(font, acts[i], (Vector2){cx+380, cy+i*rowH}, 22, 1, WHITE);
    }
}


//bot vs bot
static void DrawBotVsBot(Font font, Screen *screen)
{
    Rectangle backBtn = {20, 20, 170, 44};
    Vector2 m = GetMousePosition();
    DrawBackButton(font, m, backBtn, "< НАЗАД", screen, MODE_SELECT);

    Vector2 ts = MeasureTextEx(font, "БОТ ПРОТИВ БОТА", 52, 1);
    DrawTextEx(font, "БОТ ПРОТИВ БОТА", (Vector2){(1920-ts.x)*0.5f, 200}, 52, 1, WHITE);

    Vector2 sub = MeasureTextEx(font,"Режим в разработке...", 32, 1);
    DrawTextEx(font,"Режим в разработке...",(Vector2){(1920-sub.x)*0.5f, 540}, 32, 1, (Color){120,120,140,255});
}


// ввод seed`а
static void DrawKeyInput(Font font, Screen *screen)
{
    Rectangle backBtn = {20, 20, 170, 44};
    Vector2 m = GetMousePosition();

    DrawBackButton(font, m, backBtn, "< НАЗАД", screen, MODE_SELECT);
    Vector2 ts = MeasureTextEx(font, "ВВОД КЛЮЧА", 52, 1);
    DrawTextEx(font,"ВВОД КЛЮЧА", (Vector2){(1920-ts.x)*0.5f, 200}, 52, 1, WHITE);

    Vector2 sub = MeasureTextEx(font, "Функция в разработке...", 32, 1);
    DrawTextEx(font,"Функция в разработке...",(Vector2){(1920-sub.x)*0.5f, 540}, 32, 1, (Color){120,120,140,255});
}


//запуск
void RunMazeGenerator(void)
{
    InitWindow(1920, 1080, "Maze Generator");
    SetTargetFPS(144);

    int cp[220], n = 0;
    for (int i = 32;   i <= 126;  i++) cp[n++] = i;
    for (int i = 1040; i <= 1103; i++) cp[n++] = i;
    cp[n++] = 1025;
    cp[n++] = 1105;

    Font font = LoadFontEx(FONT_PATH, 96, cp, n);
    SetTextureFilter(font.texture, TEXTURE_FILTER_BILINEAR);

    Screen screen = MENU;
    int inputW = 0;
    int inputH = 0;
    int active = -1;
    int errorFlag = 0;
    int px = 1, py = 1;

    log_info("Application started");

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground((Color){12, 12, 22, 255});

        switch (screen)
        {
            case MENU:
            DrawMenu(font, &screen);
            break;

            case LOGIN:
            DrawLogin(font, &screen);
            break;

            case MODE_SELECT:
            DrawModeSelect(font, &screen); 
            break;

            case GAME:
            DrawGame(font, &screen, &inputW, &inputH, &active, &px, &py, &errorFlag); 
            break;

            case GENERATING: 
            DrawGenerating(font, &screen, &px, &py);
            break;

            case PLAYING:    
            DrawPlaying(font, &screen, &px, &py);
            break;

            case ABOUT:      
            DrawAbout(font, &screen);
            break;

            case HELP:       
            DrawHelp(font, &screen); 
            break;

            case BOT_VS_BOT: 
            DrawBotVsBot(font, &screen);
            break;

            case KEY_INPUT:  
            DrawKeyInput(font, &screen);
            break;
        }

        EndDrawing();
    }

    log_info("Application shutdown");
    log_close();

    free_lab();
    UnloadFont(font);
    CloseWindow();
}