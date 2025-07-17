#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "primlib.h"

#define FIELD_WIDTH    10
#define FIELD_HEIGHT   20
#define BLOCK_SIZE     30
#define PIECE_SIZE     4
#define FALL_DELAY     500
#define KEY_POLL_DELAY 20

typedef struct {
    int x;
    int y;
    int type;
    int rotation;
} Piece;

static int field[FIELD_HEIGHT][FIELD_WIDTH];

static const enum color pieceColors[7] = { RED, CYAN, MAGENTA, YELLOW, BLUE, GREEN, WHITE };

static const int pieces[7][4][PIECE_SIZE][PIECE_SIZE] = {
    {
        { {0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0} },
        { {0,0,1,0}, {0,0,1,0}, {0,0,1,0}, {0,0,1,0} },
        { {0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0} },
        { {0,1,0,0}, {0,1,0,0}, {0,1,0,0}, {0,1,0,0} }
    },
    {
        { {0,0,0,0}, {0,1,1,0}, {0,1,1,0}, {0,0,0,0} },
        { {0,0,0,0}, {0,1,1,0}, {0,1,1,0}, {0,0,0,0} },
        { {0,0,0,0}, {0,1,1,0}, {0,1,1,0}, {0,0,0,0} },
        { {0,0,0,0}, {0,1,1,0}, {0,1,1,0}, {0,0,0,0} }
    },
    {
        { {0,0,0,0}, {1,1,1,0}, {0,1,0,0}, {0,0,0,0} },
        { {0,1,0,0}, {1,1,0,0}, {0,1,0,0}, {0,0,0,0} },
        { {0,1,0,0}, {1,1,1,0}, {0,0,0,0}, {0,0,0,0} },
        { {0,1,0,0}, {0,1,1,0}, {0,1,0,0}, {0,0,0,0} }
    },
    {
        { {0,0,0,0}, {0,1,1,0}, {1,1,0,0}, {0,0,0,0} },
        { {1,0,0,0}, {1,1,0,0}, {0,1,0,0}, {0,0,0,0} },
        { {0,0,0,0}, {0,1,1,0}, {1,1,0,0}, {0,0,0,0} },
        { {1,0,0,0}, {1,1,0,0}, {0,1,0,0}, {0,0,0,0} }
    },
    {
        { {0,0,0,0}, {1,1,0,0}, {0,1,1,0}, {0,0,0,0} },
        { {0,1,0,0}, {1,1,0,0}, {1,0,0,0}, {0,0,0,0} },
        { {0,0,0,0}, {1,1,0,0}, {0,1,1,0}, {0,0,0,0} },
        { {0,1,0,0}, {1,1,0,0}, {1,0,0,0}, {0,0,0,0} }
    },
    {
        { {0,0,0,0}, {1,1,1,0}, {0,0,1,0}, {0,0,0,0} },
        { {0,1,0,0}, {0,1,0,0}, {1,1,0,0}, {0,0,0,0} },
        { {1,0,0,0}, {1,1,1,0}, {0,0,0,0}, {0,0,0,0} },
        { {0,1,1,0}, {0,1,0,0}, {0,1,0,0}, {0,0,0,0} }
    },
    {
        { {0,0,0,0}, {1,1,1,0}, {1,0,0,0}, {0,0,0,0} },
        { {1,1,0,0}, {0,1,0,0}, {0,1,0,0}, {0,0,0,0} },
        { {0,0,1,0}, {1,1,1,0}, {0,0,0,0}, {0,0,0,0} },
        { {0,1,0,0}, {0,1,0,0}, {0,1,1,0}, {0,0,0,0} }
    }
};

void initField(void) {
    for (int i = 0; i < FIELD_HEIGHT; i++)
        for (int j = 0; j < FIELD_WIDTH; j++)
            field[i][j] = 0;
}

void drawField(void) {
    int offsetX = (gfx_screenWidth() - FIELD_WIDTH * BLOCK_SIZE) / 2;
    int offsetY = (gfx_screenHeight() - FIELD_HEIGHT * BLOCK_SIZE) / 2;
    for (int i = 0; i < FIELD_HEIGHT; i++) {
        for (int j = 0; j < FIELD_WIDTH; j++) {
            int cell = field[i][j];
            enum color col = (cell == 0 ? BLACK : pieceColors[cell - 1]);
            gfx_filledRect(offsetX + j * BLOCK_SIZE + 1,
                           offsetY + i * BLOCK_SIZE + 1,
                           offsetX + (j + 1) * BLOCK_SIZE - 1,
                           offsetY + (i + 1) * BLOCK_SIZE - 1,
                           col);
        }
    }
    int offsetX2 = (gfx_screenWidth() - FIELD_WIDTH * BLOCK_SIZE) / 2;
    int offsetY2 = (gfx_screenHeight() - FIELD_HEIGHT * BLOCK_SIZE) / 2;
    for (int i = 0; i <= FIELD_HEIGHT; i++) {
        gfx_line(offsetX2, offsetY2 + i * BLOCK_SIZE,
                 offsetX2 + FIELD_WIDTH * BLOCK_SIZE, offsetY2 + i * BLOCK_SIZE,
                 WHITE);
    }
    for (int j = 0; j <= FIELD_WIDTH; j++) {
        gfx_line(offsetX2 + j * BLOCK_SIZE, offsetY2,
                 offsetX2 + j * BLOCK_SIZE, offsetY2 + FIELD_HEIGHT * BLOCK_SIZE,
                 WHITE);
    }
}

void drawPiece(const Piece *p) {
    int offsetX = (gfx_screenWidth() - FIELD_WIDTH * BLOCK_SIZE) / 2;
    int offsetY = (gfx_screenHeight() - FIELD_HEIGHT * BLOCK_SIZE) / 2;
    for (int i = 0; i < PIECE_SIZE; i++) {
        for (int j = 0; j < PIECE_SIZE; j++) {
            if (pieces[p->type][p->rotation][i][j]) {
                int boardX = p->x + j;
                int boardY = p->y + i;
                if (boardY >= 0) {
                    gfx_filledRect(offsetX + boardX * BLOCK_SIZE + 1,
                                   offsetY + boardY * BLOCK_SIZE + 1,
                                   offsetX + (boardX + 1) * BLOCK_SIZE - 1,
                                   offsetY + (boardY + 1) * BLOCK_SIZE - 1,
                                   pieceColors[p->type]);
                }
            }
        }
    }
}

bool canPlace(const Piece *p, int newX, int newY, int newRot) {
    for (int i = 0; i < PIECE_SIZE; i++) {
        for (int j = 0; j < PIECE_SIZE; j++) {
            if (pieces[p->type][newRot][i][j]) {
                int boardX = newX + j;
                int boardY = newY + i;
                if (boardX < 0 || boardX >= FIELD_WIDTH || boardY >= FIELD_HEIGHT)
                    return false;
                if (boardY >= 0 && field[boardY][boardX] != 0)
                    return false;
            }
        }
    }
    return true;
}

void mergePiece(const Piece *p) {
    for (int i = 0; i < PIECE_SIZE; i++) {
        for (int j = 0; j < PIECE_SIZE; j++) {
            if (pieces[p->type][p->rotation][i][j]) {
                int boardX = p->x + j;
                int boardY = p->y + i;
                if (boardY >= 0 && boardY < FIELD_HEIGHT && boardX >= 0 && boardX < FIELD_WIDTH)
                    field[boardY][boardX] = p->type + 1;
            }
        }
    }
}

void clearLines(int *score) {
    for (int i = FIELD_HEIGHT - 1; i >= 0; i--) {
        bool full = true;
        for (int j = 0; j < FIELD_WIDTH; j++) {
            if (field[i][j] == 0) { full = false; break; }
        }
        if (full) {
            for (int k = i; k > 0; k--) {
                for (int j = 0; j < FIELD_WIDTH; j++) {
                    field[k][j] = field[k - 1][j];
                }
            }
            for (int j = 0; j < FIELD_WIDTH; j++)
                field[0][j] = 0;
            i++;
            *score += 100;
        }
    }
}

Piece spawnPiece(void) {
    Piece newPiece;
    static int lastType = -1;
    int newType;
    do {
        newType = rand() % 7;
    } while (newType == lastType);
    lastType = newType;
    newPiece.type = newType;
    newPiece.rotation = 0;
    newPiece.x = FIELD_WIDTH / 2 - PIECE_SIZE / 2;
    newPiece.y = -2;
    return newPiece;
}

bool showGameOver(int score) {
    char buffer[100];
    sprintf(buffer, "Game Over! Your Score: %d", score);
    gfx_filledRect(0, 0, gfx_screenWidth() - 1, gfx_screenHeight() - 1, BLACK);
    gfx_textout((gfx_screenWidth() / 2) - 120, (gfx_screenHeight() / 2) - 20, buffer, WHITE);
    gfx_textout((gfx_screenWidth() / 2) - 140, (gfx_screenHeight() / 2) + 20, "Press ENTER to try again or ESC to exit", WHITE);
    gfx_updateScreen();
    int key;
    do {
        key = gfx_pollkey();
        usleep(50 * 1000);
    } while (key == -1);
    if (key == SDLK_RETURN)
        return true;
    return false;
}

int main(void) {
    srand(time(NULL));
    if (gfx_init())
        exit(3);
    while (1) {
        initField();
        int score = 0;
        Piece current = spawnPiece();
        unsigned long lastFall = SDL_GetTicks();
        bool gameOver = false;
        while (!gameOver) {
            gfx_filledRect(0, 0, gfx_screenWidth() - 1, gfx_screenHeight() - 1, BLACK);
            int key = gfx_pollkey();
            while (key != -1) {
                if (key == SDLK_LEFT && canPlace(&current, current.x - 1, current.y, current.rotation))
                    current.x--;
                else if (key == SDLK_RIGHT && canPlace(&current, current.x + 1, current.y, current.rotation))
                    current.x++;
                else if (key == SDLK_DOWN && canPlace(&current, current.x, current.y + 1, current.rotation))
                    current.y++;
                else if (key == SDLK_SPACE) {
                    int newRot = (current.rotation + 1) % 4;
                    if (canPlace(&current, current.x, current.y, newRot))
                        current.rotation = newRot;
                }
                else if (key == SDLK_RETURN) {
                    while (canPlace(&current, current.x, current.y + 1, current.rotation))
                        current.y++;
                }
                else if (key == SDLK_ESCAPE)
                    exit(0);
                key = gfx_pollkey();
            }
            unsigned long now = SDL_GetTicks();
            if (now - lastFall > FALL_DELAY) {
                if (canPlace(&current, current.x, current.y + 1, current.rotation))
                    current.y++;
                else {
                    mergePiece(&current);
                    clearLines(&score);
                    current = spawnPiece();
                    if (!canPlace(&current, current.x, current.y, current.rotation))
                        gameOver = true;
                }
                lastFall = now;
            }
            drawField();
            drawPiece(&current);
            char scoreBuffer[50];
            sprintf(scoreBuffer, "Score: %d", score);
            gfx_textout(10, 10, scoreBuffer, WHITE);
            gfx_updateScreen();
            SDL_Delay(KEY_POLL_DELAY);
        }
        if (!showGameOver(score))
            break;
    }
    return 0;
}
