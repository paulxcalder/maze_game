#include "graphic.h"
#include "labyrinth.h"
#include "error.h"
#include "db.h"
#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define FONT_PATH    "DejaVuSansBold.ttf"
#define GEN_DURATION  1.8f

/* ── Экраны ── */
typedef enum {
    MENU,
    LOGIN,
    MODE_SELECT,
    GAME,
    GENERATING,
    PLAYING,
    LEADERBOARD,
    ABOUT,
    HELP,
    KEY_INPUT
} Screen;

/* ── Логин ── */
static char login_name[33] = {0};
static int  login_len      = 0;
static int  login_error    = 0;

/* ── Лабиринт (теперь локально, не глобальные переменные из labyrinth.h) ── */
static char   **map        = NULL;
static size_l   map_width  = 0;
static size_l   map_height = 0;
static int      x_end      = -1;
static int      y_end      = -1;

/* ── Прогресс-бар генерации ── */
static float gen_timer     = 0.0f;
static int   gen_ok        = 0;
static int   show_solution = 0;

/* ── Игровой таймер / результат ── */
static long long current_seed  = 0;
static float     play_elapsed  = 0.0f;
static int       timer_running = 0;
static int       score_saved   = 0;

/* ── Экран KEY_INPUT ── */
static char key_seed_buf[22] = {0};
static int  key_seed_len     = 0;
static int  key_inputW       = 0;
static int  key_inputH       = 0;
static int  key_active       = -1;   /* 0=ширина, 1=высота, 2=seed */
static int  key_errorFlag    = 0;

/* ══════════════════════════════════════════════ вспомогательные ══ */

/* Найти позицию выхода (END) в карте */
static void find_end_pos(void)
{
    x_end = -1;
    y_end = -1;
    if (!map) return;
    for (int r = 0; r < (int)map_height; r++)
        for (int c = 0; c < (int)map_width; c++)
            if (map[r][c] == END) { x_end = c; y_end = r; return; }
}

/* Сбросить метки пути (RIGHT_WAY -> WAY) */
static void reset_path_marks(void)
{
    if (!map) return;
    for (int r = 0; r < (int)map_height; r++)
        for (int c = 0; c < (int)map_width; c++)
            if (map[r][c] == RIGHT_WAY)
                map[r][c] = WAY;
}

/* Освободить карту и обнулить все указатели */
static void free_map(void)
{
    free_lab(map, map_height);
    map        = NULL;
    map_width  = 0;
    map_height = 0;
    x_end      = -1;
    y_end      = -1;
}

/* Форматировать время MM:SS.cc */
static void fmt_time(float t, char *buf, size_t n)
{
    int   m = (int)(t / 60.0f);
    float s = t - (float)m * 60.0f;
    snprintf(buf, n, "%02d:%05.2f", m, s);
}

/* Кнопка "назад" */
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

/* ══════════════════════════════════════════════════════ экраны ══ */

/* ── МЕНЮ ── */
static void DrawMenu(Font font, Screen *screen)
{
    Vector2 ts = MeasureTextEx(font, "MAZE GENERATOR", 88, 1);
    DrawTextEx(font, "MAZE GENERATOR", (Vector2){(1920 - ts.x) * 0.5f, 110}, 88, 1, WHITE);

    Rectangle btnStart = {(1920 - 320) * 0.5f, 360, 320, 64};
    Rectangle btnHelp  = {(1920 - 320) * 0.5f, 440, 320, 64};
    Rectangle btnAbout = {(1920 - 320) * 0.5f, 520, 320, 64};
    Rectangle btnLead  = {(1920 - 320) * 0.5f, 600, 320, 64};
    Rectangle btnExit  = {(1920 - 320) * 0.5f, 680, 320, 64};

    Vector2 m = GetMousePosition();

    /* ИГРАТЬ */
    int hotStart = CheckCollisionPointRec(m, btnStart);
    DrawRectangleRec(btnStart, hotStart ? (Color){15,55,140,255} : (Color){10,40,100,255});
    DrawRectangleLinesEx(btnStart, 2, hotStart ? WHITE : (Color){60,120,255,255});
    Vector2 st = MeasureTextEx(font, "ИГРАТЬ", 28, 1);
    DrawTextEx(font, "ИГРАТЬ",
               (Vector2){btnStart.x + (btnStart.width  - st.x) * 0.5f,
                         btnStart.y + (btnStart.height - st.y) * 0.5f},
               28, 1, WHITE);
    if (hotStart && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        login_name[0] = '\0';
        login_len     = 0;
        login_error   = 0;
        *screen       = LOGIN;
    }

    /* ПОМОЩЬ */
    int hotHelp = CheckCollisionPointRec(m, btnHelp);
    DrawRectangleRec(btnHelp, hotHelp ? (Color){20,80,90,255} : (Color){14,58,68,255});
    DrawRectangleLinesEx(btnHelp, 2, hotHelp ? WHITE : (Color){40,190,210,255});
    Vector2 ht = MeasureTextEx(font, "ПОМОЩЬ", 28, 1);
    DrawTextEx(font, "ПОМОЩЬ",
               (Vector2){btnHelp.x + (btnHelp.width  - ht.x) * 0.5f,
                         btnHelp.y + (btnHelp.height - ht.y) * 0.5f},
               28, 1, WHITE);
    if (hotHelp && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) *screen = HELP;

    /* СПРАВКА */
    int hotAbout = CheckCollisionPointRec(m, btnAbout);
    DrawRectangleRec(btnAbout, hotAbout ? (Color){80,35,110,255} : (Color){60,25,85,255});
    DrawRectangleLinesEx(btnAbout, 2, hotAbout ? WHITE : (Color){170,80,255,255});
    Vector2 at = MeasureTextEx(font, "СПРАВКА", 28, 1);
    DrawTextEx(font, "СПРАВКА",
               (Vector2){btnAbout.x + (btnAbout.width  - at.x) * 0.5f,
                         btnAbout.y + (btnAbout.height - at.y) * 0.5f},
               28, 1, WHITE);
    if (hotAbout && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) *screen = ABOUT;

    /* ЛИДЕРЫ */
    int hotLead = CheckCollisionPointRec(m, btnLead);
    DrawRectangleRec(btnLead, hotLead ? (Color){100,70,15,255} : (Color){75,50,10,255});
    DrawRectangleLinesEx(btnLead, 2, hotLead ? WHITE : (Color){255,180,30,255});
    Vector2 ld = MeasureTextEx(font, "ЛИДЕРЫ", 28, 1);
    DrawTextEx(font, "ЛИДЕРЫ",
               (Vector2){btnLead.x + (btnLead.width  - ld.x) * 0.5f,
                         btnLead.y + (btnLead.height - ld.y) * 0.5f},
               28, 1, WHITE);
    if (hotLead && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) *screen = LEADERBOARD;

    /* ВЫХОД */
    int hotExit = CheckCollisionPointRec(m, btnExit);
    DrawRectangleRec(btnExit, hotExit ? (Color){120,20,20,255} : (Color){90,15,15,255});
    DrawRectangleLinesEx(btnExit, 2, hotExit ? WHITE : (Color){220,60,60,255});
    Vector2 et = MeasureTextEx(font, "ВЫХОД", 28, 1);
    DrawTextEx(font, "ВЫХОД",
               (Vector2){btnExit.x + (btnExit.width  - et.x) * 0.5f,
                         btnExit.y + (btnExit.height - et.y) * 0.5f},
               28, 1, WHITE);
    if (hotExit && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        log_info("Application closed by user via menu");
        log_close();
        CloseWindow();
    }
}

/* ── ЛОГИН ── */
static void DrawLogin(Font font, Screen *screen)
{
    Rectangle backBtn    = {20, 20, 170, 44};
    Rectangle inputBox   = {(1920 - 520) * 0.5f, 470, 520, 62};
    Rectangle confirmBtn = {(1920 - 300) * 0.5f, 582, 300, 60};

    Vector2 m = GetMousePosition();
    DrawBackButton(font, m, backBtn, "< МЕНЮ", screen, MENU);

    Vector2 ts = MeasureTextEx(font, "ВХОД В ИГРУ", 52, 1);
    DrawTextEx(font, "ВХОД В ИГРУ",
               (Vector2){(1920 - ts.x) * 0.5f, 200}, 52, 1, WHITE);
    DrawRectangle(1920/2 - 280, 268, 560, 2, (Color){60,120,255,80});

    Vector2 ss = MeasureTextEx(font, "Введите ваш уникальный логин", 26, 1);
    DrawTextEx(font, "Введите ваш уникальный логин",
               (Vector2){(1920 - ss.x) * 0.5f, 390}, 26, 1, LIGHTGRAY);

    DrawRectangleRec(inputBox, (Color){15, 15, 32, 255});
    DrawRectangleLinesEx(inputBox, 2, (Color){60, 120, 255, 255});

    static int blink_login = 0;
    blink_login++;
    char displayBuf[36];
    if ((blink_login / 36) % 2 == 0)
        snprintf(displayBuf, sizeof(displayBuf), "%s|", login_name);
    else
        snprintf(displayBuf, sizeof(displayBuf), "%s ", login_name);

    Vector2 lt = MeasureTextEx(font, displayBuf, 28, 1);
    float tx = inputBox.x + (inputBox.width - lt.x) * 0.5f;
    if (tx < inputBox.x + 10) tx = inputBox.x + 10;
    DrawTextEx(font, displayBuf,
               (Vector2){tx, inputBox.y + (inputBox.height - lt.y) * 0.5f},
               28, 1, (Color){80, 200, 120, 255});

    DrawTextEx(font, "Только латинские буквы и цифры, до 32 символов",
               (Vector2){inputBox.x, inputBox.y + inputBox.height + 10},
               16, 1, (Color){80, 80, 110, 255});

    int key = GetCharPressed();
    while (key > 0) {
        if (key > 32 && key <= 126 && login_len < 32) {
            login_name[login_len++] = (char)key;
            login_name[login_len]   = '\0';
            login_error = 0;
        }
        key = GetCharPressed();
    }
    if (IsKeyPressed(KEY_BACKSPACE) && login_len > 0) {
        login_name[--login_len] = '\0';
        login_error = 0;
    }

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
    if (doConfirm) {
        if (login_len > 0) {
            char logbuf[80];
            snprintf(logbuf, sizeof(logbuf), "User logged in: %s", login_name);
            log_info(logbuf);
            login_error = 0;
            *screen = MODE_SELECT;
        } else {
            login_error = 1;
            log_error("Login attempt with empty username");
        }
    }

    if (login_error) {
        Vector2 errt = MeasureTextEx(font, "Логин не может быть пустым!", 22, 1);
        DrawTextEx(font, "Логин не может быть пустым!",
                   (Vector2){(1920 - errt.x) * 0.5f, confirmBtn.y + confirmBtn.height + 18},
                   22, 1, RED);
    }

    if (login_len > 0 && !login_error) {
        char greet[64];
        snprintf(greet, sizeof(greet), "Привет, %s!", login_name);
        Vector2 gt = MeasureTextEx(font, greet, 20, 1);
        DrawTextEx(font, greet,
                   (Vector2){(1920 - gt.x) * 0.5f, confirmBtn.y + confirmBtn.height + 18},
                   20, 1, (Color){60, 180, 100, 200});
    }
}

/* ── ВЫБОР РЕЖИМА ── */
static void DrawModeSelect(Font font, Screen *screen)
{
    Rectangle backBtn = {20, 20, 170, 44};
    Rectangle btnSolo = {(1920 - 400) * 0.5f, 380, 400, 70};
    Rectangle btnKey  = {(1920 - 400) * 0.5f, 480, 400, 70};

    Vector2 m = GetMousePosition();

    Vector2 title = MeasureTextEx(font, "ВЫБОР РЕЖИМА", 52, 1);
    DrawTextEx(font, "ВЫБОР РЕЖИМА",
               (Vector2){(1920 - title.x) * 0.5f, 200}, 52, 1, WHITE);
    DrawRectangle(1920/2 - 300, 270, 600, 2, (Color){60,60,90,255});

    char nameLabel[48];
    snprintf(nameLabel, sizeof(nameLabel), "Игрок: %s", login_name);
    DrawTextEx(font, nameLabel, (Vector2){1920 - 400.0f, 24.0f}, 20, 1,
               (Color){80,180,120,200});

    DrawBackButton(font, m, backBtn, "< МЕНЮ", screen, MENU);

    /* В ОДИНОЧКУ */
    int hotSolo = CheckCollisionPointRec(m, btnSolo);
    DrawRectangleRec(btnSolo, hotSolo ? (Color){15,55,140,255} : (Color){10,40,100,255});
    DrawRectangleLinesEx(btnSolo, 2, hotSolo ? WHITE : (Color){60,120,255,255});
    Vector2 s1 = MeasureTextEx(font, "В ОДИНОЧКУ", 28, 1);
    DrawTextEx(font, "В ОДИНОЧКУ",
               (Vector2){btnSolo.x + (btnSolo.width  - s1.x) * 0.5f,
                         btnSolo.y + (btnSolo.height - s1.y) * 0.5f},
               28, 1, WHITE);
    if (hotSolo && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) *screen = GAME;

    /* ВВЕСТИ КЛЮЧ */
    int hotKey = CheckCollisionPointRec(m, btnKey);
    DrawRectangleRec(btnKey, hotKey ? (Color){20,80,90,255} : (Color){14,58,68,255});
    DrawRectangleLinesEx(btnKey, 2, hotKey ? WHITE : (Color){40,190,210,255});
    Vector2 s3 = MeasureTextEx(font, "ВВЕСТИ КЛЮЧ", 28, 1);
    DrawTextEx(font, "ВВЕСТИ КЛЮЧ",
               (Vector2){btnKey.x + (btnKey.width  - s3.x) * 0.5f,
                         btnKey.y + (btnKey.height - s3.y) * 0.5f},
               28, 1, WHITE);
    if (hotKey && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        /* Сбрасываем состояние экрана KEY_INPUT */
        key_seed_buf[0] = '\0';
        key_seed_len    = 0;
        key_inputW      = 0;
        key_inputH      = 0;
        key_active      = -1;
        key_errorFlag   = 0;
        *screen = KEY_INPUT;
    }
}

/* ── НАСТРОЙКА ЛАБИРИНТА (случайный seed) ── */
static void DrawGame(Font font, Screen *screen,
                     int *inputW, int *inputH,
                     int *active, int *px, int *py, int *errorFlag)
{
    Rectangle backBtn  = {20, 20, 170, 44};
    Rectangle widthBox = {384, 238, 220, 52};
    Rectangle heightBox= {1152, 238, 220, 52};
    Rectangle genBtn   = {810, 410, 300, 62};

    Vector2 m = GetMousePosition();

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (CheckCollisionPointRec(m, widthBox))       { *active = 0; }
        else if (CheckCollisionPointRec(m, heightBox)) { *active = 1; }
        else if (CheckCollisionPointRec(m, genBtn)) {
            if (*inputW >= 3 && *inputH >= 3) {
                *errorFlag    = 0;
                current_seed  = (long long)time(NULL);

                free_map();
                map = generate_lab(current_seed, (size_l)*inputW, (size_l)*inputH);

                if (map != NULL) {
                    map_width  = (size_l)*inputW;
                    map_height = (size_l)*inputH;
                    find_end_pos();

                    char logbuf[80];
                    snprintf(logbuf, sizeof(logbuf),
                             "Labyrinth %dx%d generated, seed=%lld, user=%s",
                             *inputW, *inputH, current_seed, login_name);
                    log_info(logbuf);

                    show_solution = 0;
                    gen_ok        = 1;
                    gen_timer     = 0.0f;
                    *px = 1; *py = 1;
                    *screen = GENERATING;
                } else {
                    log_error("DrawGame: generate_lab returned NULL");
                    *errorFlag = 2;
                }
            } else {
                log_error("DrawGame: labyrinth size < 3 requested");
                *errorFlag = 1;
            }
        } else if (CheckCollisionPointRec(m, backBtn)) {
            *screen = MODE_SELECT;
        } else {
            *active = -1;
        }
    }

    /* Ввод цифр */
    int key = GetCharPressed();
    while (key > 0) {
        if (key >= '0' && key <= '9') {
            if (*active == 0) { int v = (*inputW)*10+(key-'0'); *inputW = (v>100)?0:v; }
            if (*active == 1) { int v = (*inputH)*10+(key-'0'); *inputH = (v>100)?0:v; }
        }
        key = GetCharPressed();
    }
    if (IsKeyPressed(KEY_BACKSPACE)) {
        if (*active == 0) *inputW /= 10;
        if (*active == 1) *inputH /= 10;
    }

    DrawBackButton(font, m, backBtn, "< НАЗАД", screen, MODE_SELECT);

    Vector2 titleV = MeasureTextEx(font, "НАСТРОЙКА ЛАБИРИНТА", 42, 1);
    DrawTextEx(font, "НАСТРОЙКА ЛАБИРИНТА",
               (Vector2){(1920 - titleV.x) * 0.5f, 86}, 42, 1, WHITE);

    DrawTextEx(font, "ШИРИНА",    (Vector2){widthBox.x,  widthBox.y  - 26}, 18, 1, LIGHTGRAY);
    DrawTextEx(font, "ВЫСОТА",    (Vector2){heightBox.x, heightBox.y - 26}, 18, 1, LIGHTGRAY);
    DrawTextEx(font, "(макс. 100)",(Vector2){widthBox.x,  widthBox.y  + 58}, 16, 1, (Color){100,100,130,255});
    DrawTextEx(font, "(макс. 100)",(Vector2){heightBox.x, heightBox.y + 58}, 16, 1, (Color){100,100,130,255});

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

    if (*errorFlag == 1) {
        Vector2 errt = MeasureTextEx(font, "Размер должен быть не менее 3!", 22, 1);
        DrawTextEx(font, "Размер должен быть не менее 3!",
                   (Vector2){(1920 - errt.x) * 0.5f, 500}, 22, 1, RED);
    } else if (*errorFlag == 2) {
        Vector2 errt = MeasureTextEx(font,
            "Не удалось создать лабиринт. Попробуйте другой размер.", 22, 1);
        DrawTextEx(font,
            "Не удалось создать лабиринт. Попробуйте другой размер.",
            (Vector2){(1920 - errt.x) * 0.5f, 500}, 22, 1, RED);
    }
}

/* ── ПРОГРЕСС-БАР ── */
static void DrawGenerating(Font font, Screen *screen, int *px, int *py)
{
    gen_timer += GetFrameTime();

    float progress = gen_timer / GEN_DURATION;
    if (progress > 1.0f) progress = 1.0f;

    Vector2 ts = MeasureTextEx(font, "ГЕНЕРАЦИЯ ЛАБИРИНТА", 48, 1);
    DrawTextEx(font, "ГЕНЕРАЦИЯ ЛАБИРИНТА",
               (Vector2){(1920 - ts.x) * 0.5f, 340}, 48, 1, WHITE);

    char nameLabel[48];
    snprintf(nameLabel, sizeof(nameLabel), "Игрок: %s", login_name);
    Vector2 nl = MeasureTextEx(font, nameLabel, 22, 1);
    DrawTextEx(font, nameLabel,
               (Vector2){(1920 - nl.x) * 0.5f, 412}, 22, 1, (Color){80,180,120,210});

    float barW = 720.0f, barH = 46.0f;
    float barX = (1920 - barW) * 0.5f;
    float barY = 490.0f;

    DrawRectangle((int)barX, (int)barY, (int)barW, (int)barH, (Color){22, 22, 42, 255});

    int fillW = (int)(barW * progress);
    if (fillW > 0)
        DrawRectangleGradientH((int)barX, (int)barY, fillW, (int)barH,
                               (Color){0, 140, 60, 255}, (Color){0, 220, 100, 255});
    if (fillW > 4)
        DrawRectangle((int)barX + 2, (int)barY + 2, fillW - 4, 7,
                      (Color){180, 255, 200, 35});

    DrawRectangleLinesEx((Rectangle){barX, barY, barW, barH}, 2, (Color){55,55,88,255});

    char pctText[8];
    snprintf(pctText, sizeof(pctText), "%d%%", (int)(progress * 100.0f));
    Vector2 pt = MeasureTextEx(font, pctText, 22, 1);
    DrawTextEx(font, pctText,
               (Vector2){barX + (barW - pt.x) * 0.5f, barY + (barH - pt.y) * 0.5f},
               22, 1, WHITE);

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

    /* Показываем seed игроку */
    char seedInfo[48];
    snprintf(seedInfo, sizeof(seedInfo), "Seed: %lld", current_seed);
    Vector2 si = MeasureTextEx(font, seedInfo, 18, 1);
    DrawTextEx(font, seedInfo,
               (Vector2){(1920 - si.x) * 0.5f, barY + barH + 60},
               18, 1, (Color){100, 100, 130, 200});

    if (gen_timer >= GEN_DURATION) {
        if (gen_ok) {
            reset_path_marks();
            show_solution  = 0;
            play_elapsed   = 0.0f;
            timer_running  = 1;
            score_saved    = 0;
            *px = 1; *py = 1;
            log_info("Entering PLAYING screen");
            *screen = PLAYING;
        } else {
            log_error("DrawGenerating: gen_ok=0 on finish, returning to GAME");
            *screen = GAME;
        }
    }
}

/* ── ИГРА ── */
static void DrawPlaying(Font font, Screen *screen, int *px, int *py)
{
    if (map == NULL) { *screen = GAME; return; }

    int won = (*px == x_end && *py == y_end);

    if (timer_running && !won) play_elapsed += GetFrameTime();
    if (won && timer_running)  timer_running = 0;

    /* Движение */
    if (!won) {
        int nx = *px, ny = *py;
        if      (IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP))    ny--;
        else if (IsKeyPressed(KEY_S) || IsKeyPressed(KEY_DOWN))   ny++;
        else if (IsKeyPressed(KEY_A) || IsKeyPressed(KEY_LEFT))   nx--;
        else if (IsKeyPressed(KEY_D) || IsKeyPressed(KEY_RIGHT))  nx++;

        if ((nx != *px || ny != *py) &&
            nx >= 0 && nx < (int)map_width  &&
            ny >= 0 && ny < (int)map_height &&
            map[ny][nx] != WALL)
        {
            *px = nx; *py = ny;
        }

        if (IsKeyPressed(KEY_R)) {
            reset_path_marks();
            show_solution = 0;
            *px = 1; *py = 1;
        }
        if (IsKeyPressed(KEY_M)) {
            reset_path_marks();
            show_solution = 0;
            *screen = GAME;
            return;
        }
    }

    /* Размер клетки */
    float cellW    = 1880.0f / (float)map_width;
    float cellH    = 1020.0f / (float)map_height;
    float cellSize = (cellW < cellH) ? cellW : cellH;
    if (cellSize > 40.0f) cellSize = 40.0f;
    if (cellSize < 1.0f)  cellSize = 1.0f;

    float mazePixW = cellSize * (float)map_width;
    float mazePixH = cellSize * (float)map_height;
    float offX     = (1920.0f - mazePixW) * 0.5f;
    float offY     = (1080.0f - mazePixH) * 0.5f;

    /* Отрисовка карты */
    for (int r = 0; r < (int)map_height; r++) {
        for (int c = 0; c < (int)map_width; c++) {
            Color col;
            if      (map[r][c] == WALL)      col = (Color){38, 38, 62, 255};
            else if (show_solution && map[r][c] == RIGHT_WAY)
                                             col = (Color){160, 15, 15, 255};
            else                             col = (Color){8,  8,  20, 255};

            DrawRectangleV(
                (Vector2){offX + c * cellSize, offY + r * cellSize},
                (Vector2){cellSize, cellSize},
                col);
        }
    }

    /* Старт */
    DrawRectangleV((Vector2){offX + 1 * cellSize, offY + 1 * cellSize},
                   (Vector2){cellSize, cellSize}, (Color){20, 80, 180, 120});

    /* Выход */
    DrawRectangleV((Vector2){offX + x_end * cellSize, offY + y_end * cellSize},
                   (Vector2){cellSize, cellSize}, (Color){0, 200, 80, 255});
    if (cellSize >= 10.0f) {
        Vector2 ev = MeasureTextEx(font, "В", cellSize * 0.55f, 1);
        DrawTextEx(font, "В",
                   (Vector2){offX + x_end * cellSize + (cellSize - ev.x) * 0.5f,
                             offY + y_end * cellSize + (cellSize - ev.y) * 0.5f},
                   cellSize * 0.55f, 1, (Color){0, 255, 100, 255});
    }

    /* Игрок */
    float margin = cellSize * 0.12f;
    float ps     = cellSize - 2.0f * margin;
    DrawRectangleV(
        (Vector2){offX + *px * cellSize + margin, offY + *py * cellSize + margin},
        (Vector2){ps, ps},
        (Color){80, 140, 255, 255});

    /* Таймер (вверху по центру) */
    char tbuf[20], tdisp[40];
    fmt_time(play_elapsed, tbuf, sizeof(tbuf));
    snprintf(tdisp, sizeof(tdisp), "Время: %s", tbuf);
    Vector2 td = MeasureTextEx(font, tdisp, 20, 1);
    DrawTextEx(font, tdisp,
               (Vector2){(1920.0f - td.x) * 0.5f, 12.0f},
               20, 1, (Color){200, 200, 220, 220});

    /* Имя игрока (верхний правый угол) */
    char nameLabel[48];
    snprintf(nameLabel, sizeof(nameLabel), "Игрок: %s", login_name);
    DrawTextEx(font, nameLabel, (Vector2){1920.0f - 300.0f, 12.0f},
               18, 1, (Color){80, 180, 120, 180});

    /* Seed (верхний левый, рядом с кнопкой меню) */
    char seedLabel[48];
    snprintf(seedLabel, sizeof(seedLabel), "Seed: %lld", current_seed);
    DrawTextEx(font, seedLabel, (Vector2){210.0f, 18.0f},
               16, 1, (Color){100, 100, 130, 200});

    /* Кнопка МЕНЮ (верхний левый) */
    Rectangle menuBtn = {20.0f, 14.0f, 170.0f, 40.0f};
    Vector2 m = GetMousePosition();
    int hotMenu = CheckCollisionPointRec(m, menuBtn);
    DrawRectangleRec(menuBtn, hotMenu ? (Color){55,55,68,255} : (Color){35,35,48,255});
    DrawRectangleLinesEx(menuBtn, 2, hotMenu ? WHITE : GRAY);
    Vector2 mt = MeasureTextEx(font, "< МЕНЮ", 18, 1);
    DrawTextEx(font, "< МЕНЮ",
               (Vector2){menuBtn.x + (menuBtn.width  - mt.x) * 0.5f,
                         menuBtn.y + (menuBtn.height - mt.y) * 0.5f},
               18, 1, LIGHTGRAY);
    if (hotMenu && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        reset_path_marks();
        show_solution = 0;
        *screen = MENU;
        return;
    }

    /* ── Кнопка АВТО-ПРОХОД (правый нижний угол) ── */
    Rectangle autoBtn = {1920.0f - 300.0f, 1030.0f, 280.0f, 42.0f};
    int hotAuto = CheckCollisionPointRec(m, autoBtn);
    Color bg = show_solution ? (Color){120,20,20,255} : (Color){70,15,15,255};
    if (hotAuto) bg = show_solution ? (Color){160,30,30,255} : (Color){100,20,20,255};
    DrawRectangleRec(autoBtn, bg);
    DrawRectangleLinesEx(autoBtn, 2, hotAuto ? WHITE : (Color){220,60,60,255});
    const char *autoLabel = show_solution ? "СКРЫТЬ ПУТЬ" : "АВТО-ПРОХОД";
    Vector2 alt = MeasureTextEx(font, autoLabel, 20, 1);
    DrawTextEx(font, autoLabel,
               (Vector2){autoBtn.x + (autoBtn.width  - alt.x) * 0.5f,
                         autoBtn.y + (autoBtn.height - alt.y) * 0.5f},
               20, 1, WHITE);
    if (hotAuto && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        show_solution = !show_solution;
        if (show_solution)
            find_way(map, 1, 1, 1);   /* отметить путь RIGHT_WAY */
        else
            reset_path_marks();        /* снять метки */
    }

    /* Подсказка (нижний левый) */
    DrawTextEx(font,
               "WASD / стрелки - движение     R - рестарт     M - настройки",
               (Vector2){20.0f, 1056.0f}, 16, 1, (Color){70, 70, 100, 255});

    /* ── Победа ── */
    if (won) {
        if (!score_saved) {
            db_save(login_name, play_elapsed, current_seed);
            score_saved = 1;
        }

        DrawRectangle(0, 0, 1920, 1080, (Color){0, 0, 0, 170});

        const char *msgWin = "ВЫХОД НАЙДЕН!";
        Vector2 wt = MeasureTextEx(font, msgWin, 80, 1);
        DrawTextEx(font, msgWin,
                   (Vector2){(1920.0f - wt.x) * 0.5f, 420.0f},
                   80, 1, (Color){0, 220, 80, 255});

        char winTime[32], winLine[64];
        fmt_time(play_elapsed, winTime, sizeof(winTime));
        snprintf(winLine, sizeof(winLine), "Ваше время: %s", winTime);
        Vector2 wlt = MeasureTextEx(font, winLine, 30, 1);
        DrawTextEx(font, winLine,
                   (Vector2){(1920.0f - wlt.x) * 0.5f, 520.0f},
                   30, 1, (Color){255, 215, 50, 255});

        char seedLine[48];
        snprintf(seedLine, sizeof(seedLine), "Seed: %lld", current_seed);
        Vector2 sl = MeasureTextEx(font, seedLine, 22, 1);
        DrawTextEx(font, seedLine,
                   (Vector2){(1920.0f - sl.x) * 0.5f, 560.0f},
                   22, 1, (Color){150, 150, 170, 200});

        const char *msgSub = "R - новый лабиринт     M - к настройкам";
        Vector2 st = MeasureTextEx(font, msgSub, 30, 1);
        DrawTextEx(font, msgSub,
                   (Vector2){(1920.0f - st.x) * 0.5f, 600.0f},
                   30, 1, LIGHTGRAY);

        /* R — новый лабиринт того же размера, новый seed */
        if (IsKeyPressed(KEY_R)) {
            score_saved   = 0;
            show_solution = 0;
            current_seed  = (long long)time(NULL);
            free_map();
            map = generate_lab(current_seed, map_width, map_height);
            if (map != NULL) {
                find_end_pos();
                gen_ok    = 1;
                gen_timer = 0.0f;
                *screen   = GENERATING;
            } else {
                *screen = GAME;
            }
        }

        /* M — к настройкам */
        if (IsKeyPressed(KEY_M)) {
            score_saved   = 0;
            show_solution = 0;
            reset_path_marks();
            *screen = GAME;
        }
    }
}

/* ── ЛИДЕРБОРД (из файла) ── */
static void DrawLeaderboard(Font font, Screen *screen)
{
    Rectangle backBtn = {20, 20, 170, 44};
    Vector2 m = GetMousePosition();
    DrawBackButton(font, m, backBtn, "< МЕНЮ", screen, MENU);

    Vector2 ts = MeasureTextEx(font, "ЛИДЕРБОРД", 52, 1);
    DrawTextEx(font, "ЛИДЕРБОРД",
               (Vector2){(1920 - ts.x) * 0.5f, 76}, 52, 1, WHITE);
    DrawRectangle(1920 / 2 - 320, 148, 640, 2, (Color){255,180,30,100});

    /* Загружаем данные из файла */
    static DBEntry entries[DB_MAX_ENTRIES];
    int count = db_load(entries, DB_MAX_ENTRIES);

    if (count == 0) {
        Vector2 es = MeasureTextEx(font, "Пока нет записей. Пройдите лабиринт!", 30, 1);
        DrawTextEx(font, "Пока нет записей. Пройдите лабиринт!",
                   (Vector2){(1920 - es.x) * 0.5f, 510}, 30, 1, (Color){120,120,140,255});
        return;
    }

    /* Показываем не более 10 лучших */
    int show = count < 10 ? count : 10;

    float startY = 200.0f;
    float rowH   = 56.0f;
    float col1   = 130.0f, col2 = 300.0f, col3 = 1080.0f, col4 = 1480.0f;

    /* Заголовок таблицы */
    DrawRectangle(100, (int)startY, 1720, (int)rowH, (Color){28,28,52,255});
    DrawTextEx(font, "#",      (Vector2){col1, startY + 16}, 22, 1, (Color){255,180,30,255});
    DrawTextEx(font, "Имя",    (Vector2){col2, startY + 16}, 22, 1, (Color){255,180,30,255});
    DrawTextEx(font, "Время",  (Vector2){col3, startY + 16}, 22, 1, (Color){255,180,30,255});
    DrawTextEx(font, "Seed",   (Vector2){col4, startY + 16}, 22, 1, (Color){255,180,30,255});
    DrawRectangle(100, (int)(startY + rowH), 1720, 2, (Color){80,80,110,255});

    for (int i = 0; i < show; i++) {
        float ry   = startY + rowH + 4.0f + (float)i * (rowH + 2.0f);
        Color rowBg = (i % 2 == 0) ? (Color){14,14,28,220} : (Color){22,22,42,220};
        DrawRectangle(100, (int)ry, 1720, (int)rowH, rowBg);

        Color nc = WHITE;
        if (i == 0) nc = (Color){255,215,0,255};
        if (i == 1) nc = (Color){192,192,192,255};
        if (i == 2) nc = (Color){205,127,50,255};

        char rankBuf[8], timeBuf[20], seedBuf[28];
        snprintf(rankBuf, sizeof(rankBuf), "%d", i + 1);
        fmt_time(entries[i].time_sec, timeBuf, sizeof(timeBuf));
        snprintf(seedBuf, sizeof(seedBuf), "%lld", entries[i].seed);

        DrawTextEx(font, rankBuf,          (Vector2){col1, ry + 16}, 22, 1, nc);
        DrawTextEx(font, entries[i].name,  (Vector2){col2, ry + 16}, 22, 1, WHITE);
        DrawTextEx(font, timeBuf,          (Vector2){col3, ry + 16}, 22, 1, (Color){80,220,120,255});
        DrawTextEx(font, seedBuf,          (Vector2){col4, ry + 16}, 22, 1, (Color){150,150,170,255});
    }
}

/* ── СПРАВКА ── */
static void DrawAbout(Font font, Screen *screen)
{
    Rectangle backBtn = {20, 20, 170, 44};
    Vector2 m = GetMousePosition();
    DrawBackButton(font, m, backBtn, "< МЕНЮ", screen, MENU);

    Vector2 ts = MeasureTextEx(font, "СПРАВКА", 52, 1);
    DrawTextEx(font, "СПРАВКА", (Vector2){(1920-ts.x)*0.5f, 76}, 52, 1, WHITE);
    DrawRectangle(1920/2-280, 194, 560, 2, (Color){80,80,110,255});

    DrawTextEx(font, "Авторы:",  (Vector2){1920*0.15f,       248}, 22, 1, GRAY);
    DrawTextEx(font, "Абдульманов Данияр Айдарович",
                                 (Vector2){1920*0.15f + 200,  248}, 22, 1, WHITE);
    DrawTextEx(font, "Сергунин Роман Дмитриевич",
                                 (Vector2){1920*0.15f + 200,  284}, 22, 1, WHITE);
    DrawTextEx(font, "Группа:",  (Vector2){1920*0.15f,        338}, 22, 1, GRAY);
    DrawTextEx(font, "5131001/50601",
                                 (Vector2){1920*0.15f + 200,  338}, 22, 1, WHITE);
    DrawTextEx(font, "Год:",     (Vector2){1920*0.15f,        392}, 22, 1, GRAY);
    DrawTextEx(font, "2026",     (Vector2){1920*0.15f + 200,  392}, 22, 1, WHITE);
    DrawTextEx(font, "ВУЗ:",     (Vector2){1920*0.15f,        446}, 22, 1, GRAY);
    DrawTextEx(font, "СПбГУ им. Петра Великого",
                                 (Vector2){1920*0.15f + 200,  446}, 22, 1, WHITE);
    DrawTextEx(font, "Институт:",(Vector2){1920*0.15f,        500}, 22, 1, GRAY);
    DrawTextEx(font, "Институт компьютерных наук и кибербезопасности",
                                 (Vector2){1920*0.15f + 200,  500}, 22, 1, WHITE);
    DrawTextEx(font, "Кафедра:", (Vector2){1920*0.15f,        554}, 22, 1, GRAY);
    DrawTextEx(font, "Высшая Школа Кибербезопасности",
                                 (Vector2){1920*0.15f + 200,  554}, 22, 1, WHITE);
}

/* ── ПОМОЩЬ ── */
static void DrawHelp(Font font, Screen *screen)
{
    Rectangle backBtn = {20, 20, 170, 44};
    Vector2 m = GetMousePosition();
    DrawBackButton(font, m, backBtn, "< МЕНЮ", screen, MENU);

    Vector2 ts = MeasureTextEx(font, "ПОМОЩЬ", 52, 1);
    DrawTextEx(font, "ПОМОЩЬ", (Vector2){(1920-ts.x)*0.5f, 76}, 52, 1, WHITE);
    DrawRectangle(1920/2-320, 148, 640, 2, (Color){40,190,210,100});

    DrawTextEx(font, "ПРАВИЛА ИГРЫ",
               (Vector2){1920*0.12f, 180}, 28, 1, (Color){40,190,210,255});
    DrawRectangle(1920*0.12f, 216, 700, 1, (Color){60,60,90,255});
    DrawTextEx(font,
        "Цель - провести своего персонажа от стартовой точки до выхода из лабиринта.",
        (Vector2){1920*0.12f, 230}, 22, 1, WHITE);
    DrawTextEx(font,
        "Лабиринт генерируется случайно, но всегда имеет единственное решение.",
        (Vector2){1920*0.12f, 268}, 22, 1, WHITE);
    DrawTextEx(font,
        "Размер лабиринта задаётся вручную: ширина и высота от 3 до 100 клеток.",
        (Vector2){1920*0.12f, 306}, 22, 1, (Color){255,220,80,255});
    DrawTextEx(font,
        "Seed (ключ) позволяет воссоздать точно такой же лабиринт.",
        (Vector2){1920*0.12f, 344}, 22, 1, (Color){180,180,180,255});

    DrawTextEx(font, "УПРАВЛЕНИЕ",
               (Vector2){1920*0.12f, 410}, 28, 1, (Color){40,190,210,255});
    DrawRectangle(1920*0.12f, 446, 700, 1, (Color){60,60,90,255});

    float cx = 1920*0.12f, cy = 460, rowH = 46;
    const char *keys[] = {
        "W / Стрелка вверх", "S / Стрелка вниз",
        "A / Стрелка влево", "D / Стрелка вправо",
        "R", "M"
    };
    const char *acts[] = {
        "Движение вверх",   "Движение вниз",
        "Движение влево",   "Движение вправо",
        "Рестарт (вернуться на старт)",
        "Вернуться к настройкам"
    };
    for (int i = 0; i < 6; i++) {
        Color rowColor = (i%2==0) ? (Color){20,20,36,200} : (Color){28,28,52,200};
        DrawRectangle((int)cx-10, (int)(cy+i*rowH)-4, 900, (int)rowH-2, rowColor);
        DrawTextEx(font, keys[i], (Vector2){cx,     cy+i*rowH}, 22, 1, (Color){100,220,255,255});
        DrawTextEx(font, "-",     (Vector2){cx+340, cy+i*rowH}, 22, 1, GRAY);
        DrawTextEx(font, acts[i], (Vector2){cx+380, cy+i*rowH}, 22, 1, WHITE);
    }
}

/* ── ВВОД КЛЮЧА (SEED + размеры) ── */
static void DrawKeyInput(Font font, Screen *screen, int *px, int *py)
{
    Rectangle backBtn   = { 20,   20, 170,  44};
    Rectangle widthBox  = { 490, 440, 220,  52};
    Rectangle heightBox = { 760, 440, 220,  52};
    Rectangle seedBox   = {1030, 440, 380,  52};
    Rectangle genBtn    = {(1920-300)*0.5f, 560, 300, 62};

    Vector2 m = GetMousePosition();

    /* Выбор активного поля */
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if      (CheckCollisionPointRec(m, widthBox))  key_active = 0;
        else if (CheckCollisionPointRec(m, heightBox)) key_active = 1;
        else if (CheckCollisionPointRec(m, seedBox))   key_active = 2;
        else if (!CheckCollisionPointRec(m, genBtn) &&
                 !CheckCollisionPointRec(m, backBtn))  key_active = -1;
    }

    /* Ввод цифр */
    int key = GetCharPressed();
    while (key > 0) {
        if (key >= '0' && key <= '9') {
            if (key_active == 0) {
                int v = key_inputW * 10 + (key - '0');
                key_inputW = (v > 100) ? 0 : v;
            } else if (key_active == 1) {
                int v = key_inputH * 10 + (key - '0');
                key_inputH = (v > 100) ? 0 : v;
            } else if (key_active == 2 && key_seed_len < 20) {
                key_seed_buf[key_seed_len++] = (char)key;
                key_seed_buf[key_seed_len]   = '\0';
                key_errorFlag = 0;
            }
        }
        key = GetCharPressed();
    }
    if (IsKeyPressed(KEY_BACKSPACE)) {
        if      (key_active == 0 && key_inputW > 0) key_inputW /= 10;
        else if (key_active == 1 && key_inputH > 0) key_inputH /= 10;
        else if (key_active == 2 && key_seed_len > 0)
            key_seed_buf[--key_seed_len] = '\0';
    }

    DrawBackButton(font, m, backBtn, "< НАЗАД", screen, MODE_SELECT);

    /* Заголовок */
    Vector2 ts = MeasureTextEx(font, "ВВОД КЛЮЧА", 52, 1);
    DrawTextEx(font, "ВВОД КЛЮЧА",
               (Vector2){(1920-ts.x)*0.5f, 200}, 52, 1, WHITE);
    DrawRectangle(1920/2-280, 272, 560, 2, (Color){40,190,210,80});

    Vector2 ss = MeasureTextEx(font, "Введите seed и размеры — получите тот же лабиринт", 26, 1);
    DrawTextEx(font, "Введите seed и размеры — получите тот же лабиринт",
               (Vector2){(1920-ss.x)*0.5f, 310}, 26, 1, LIGHTGRAY);

    /* Метки полей */
    DrawTextEx(font, "ШИРИНА",      (Vector2){widthBox.x,  widthBox.y  - 28}, 18, 1, LIGHTGRAY);
    DrawTextEx(font, "ВЫСОТА",      (Vector2){heightBox.x, heightBox.y - 28}, 18, 1, LIGHTGRAY);
    DrawTextEx(font, "КЛЮЧ (SEED)", (Vector2){seedBox.x,   seedBox.y   - 28}, 18, 1, LIGHTGRAY);

    /* Рамки полей */
    DrawRectangleLinesEx(widthBox,  2, key_active==0 ? BLUE : GRAY);
    DrawRectangleLinesEx(heightBox, 2, key_active==1 ? BLUE : GRAY);
    DrawRectangleLinesEx(seedBox,   2, key_active==2 ? BLUE : GRAY);

    /* Значения числовых полей */
    DrawTextEx(font, TextFormat("%d", key_inputW),
               (Vector2){widthBox.x  + 12, widthBox.y  + 13}, 26, 1, GREEN);
    DrawTextEx(font, TextFormat("%d", key_inputH),
               (Vector2){heightBox.x + 12, heightBox.y + 13}, 26, 1, GREEN);

    /* Поле seed с мигающим курсором */
    static int blink_key = 0;
    blink_key++;
    char seedDisplay[24];
    if (key_active == 2 && (blink_key / 36) % 2 == 0)
        snprintf(seedDisplay, sizeof(seedDisplay), "%s|", key_seed_buf);
    else
        snprintf(seedDisplay, sizeof(seedDisplay), "%s",  key_seed_buf);
    DrawTextEx(font, seedDisplay,
               (Vector2){seedBox.x + 12, seedBox.y + 13},
               26, 1, (Color){80, 200, 120, 255});

    /* Подсказки под полями */
    DrawTextEx(font, "(макс. 100)",
               (Vector2){widthBox.x,  widthBox.y  + 58}, 16, 1, (Color){100,100,130,255});
    DrawTextEx(font, "(макс. 100)",
               (Vector2){heightBox.x, heightBox.y + 58}, 16, 1, (Color){100,100,130,255});
    DrawTextEx(font, "(до 20 цифр)",
               (Vector2){seedBox.x,   seedBox.y   + 58}, 16, 1, (Color){100,100,130,255});

    /* Кнопка генерации */
    int hotGen = CheckCollisionPointRec(m, genBtn);
    DrawRectangleRec(genBtn, hotGen ? (Color){15,80,28,255} : (Color){8,55,18,255});
    DrawRectangleLinesEx(genBtn, 2, hotGen ? WHITE : (Color){50,200,80,255});
    Vector2 gt = MeasureTextEx(font, "СГЕНЕРИРОВАТЬ", 24, 1);
    DrawTextEx(font, "СГЕНЕРИРОВАТЬ",
               (Vector2){genBtn.x + (genBtn.width  - gt.x) * 0.5f,
                         genBtn.y + (genBtn.height - gt.y) * 0.5f},
               24, 1, WHITE);

    int doGen = (hotGen && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
              || IsKeyPressed(KEY_ENTER);
    if (doGen) {
        if (key_inputW < 3 || key_inputH < 3) {
            key_errorFlag = 1;
        } else if (key_seed_len == 0) {
            key_errorFlag = 2;
        } else {
            key_errorFlag = 0;
            long long seed_val = atoll(key_seed_buf);
            free_map();
            map = generate_lab(seed_val, (size_l)key_inputW, (size_l)key_inputH);
            if (map != NULL) {
                map_width    = (size_l)key_inputW;
                map_height   = (size_l)key_inputH;
                current_seed = seed_val;
                find_end_pos();

                char logbuf[80];
                snprintf(logbuf, sizeof(logbuf),
                         "KeyInput: maze %dx%d seed=%lld user=%s",
                         key_inputW, key_inputH, seed_val, login_name);
                log_info(logbuf);

                show_solution = 0;
                gen_ok        = 1;
                gen_timer     = 0.0f;
                *px = 1; *py = 1;
                *screen = GENERATING;
            } else {
                key_errorFlag = 3;
                log_error("KeyInput: generate_lab returned NULL");
            }
        }
    }

    /* Ошибки */
    const char *errMsg = NULL;
    if      (key_errorFlag == 1) errMsg = "Размер должен быть не менее 3!";
    else if (key_errorFlag == 2) errMsg = "Введите seed!";
    else if (key_errorFlag == 3) errMsg = "Не удалось создать лабиринт. Попробуйте другой seed.";

    if (errMsg) {
        Vector2 et = MeasureTextEx(font, errMsg, 22, 1);
        DrawTextEx(font, errMsg,
                   (Vector2){(1920 - et.x) * 0.5f, 670}, 22, 1, RED);
    }
}

/* ══════════════════════════════════════════════════ главный цикл ══ */

void RunMazeGenerator(void)
{
    InitWindow(1920, 1080, "Maze Generator");
    SetExitKey(KEY_NULL);
    SetTargetFPS(144);

    /* Кодовые точки: ASCII + кириллица */
    int cp[220], n = 0;
    for (int i = 32;   i <= 126;  i++) cp[n++] = i;
    for (int i = 1040; i <= 1103; i++) cp[n++] = i;
    cp[n++] = 1025;
    cp[n++] = 1105;

    Font font = LoadFontEx(FONT_PATH, 96, cp, n);
    SetTextureFilter(font.texture, TEXTURE_FILTER_BILINEAR);

    Screen screen   = MENU;
    int    inputW   = 0;
    int    inputH   = 0;
    int    active   = -1;
    int    errorFlag= 0;
    int    px       = 1, py = 1;

    log_info("Application started");

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground((Color){12, 12, 22, 255});

        switch (screen) {
            case MENU:       DrawMenu(font, &screen);                               break;
            case LOGIN:      DrawLogin(font, &screen);                              break;
            case MODE_SELECT:DrawModeSelect(font, &screen);                         break;
            case GAME:       DrawGame(font, &screen, &inputW, &inputH,
                                      &active, &px, &py, &errorFlag);              break;
            case GENERATING: DrawGenerating(font, &screen, &px, &py);              break;
            case PLAYING:    DrawPlaying(font, &screen, &px, &py);                 break;
            case LEADERBOARD:DrawLeaderboard(font, &screen);                        break;
            case ABOUT:      DrawAbout(font, &screen);                              break;
            case HELP:       DrawHelp(font, &screen);                               break;
            case KEY_INPUT:  DrawKeyInput(font, &screen, &px, &py);                break;
        }

        EndDrawing();
    }

    log_info("Application shutdown");
    log_close();

    free_map();
    UnloadFont(font);
    CloseWindow();
}