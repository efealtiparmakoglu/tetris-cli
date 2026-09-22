/* tetris.h — Tetris core: board, SRS rotation, 7-bag, scoring.
 * Pure logic; no terminal I/O here (see main.c). */
#ifndef TETRIS_H
#define TETRIS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define BOARD_W 10
#define BOARD_H 20
#define NUM_PIECES 7

typedef enum { I, J, L, O, S, T, Z } PieceKind;

typedef struct {
    /* occupied cells per rotation state; each rotation lists 4 (x,y)
     * offsets from the piece origin, y grows downward */
    int cells[4][4][2];
    /* SRS kick offsets per transition; dy is in board orientation
     * (positive = downward), already converted from SRS's up-positive */
    int kicks[8][5][2];
} PieceDef;

typedef struct {
    int board[BOARD_H][BOARD_W]; /* 0 empty, 1..7 piece kind + 1 */
    int kind;                    /* current piece: PieceKind */
    int rot;                     /* 0..3 */
    int x, y;                    /* piece origin on the board */
    int nextQueue[NUM_PIECES * 2]; /* upcoming kinds: current bag + next */
    int bagPos;
    uint64_t rngState;
    int holdPiece;               /* -1 none, else PieceKind */
    bool holdUsed;
    int lines, level, score;
    bool gameOver;
} Game;

extern const PieceDef PIECES[NUM_PIECES];

void game_init(Game *g, uint64_t seed);
/* actions return true when the board changed */
bool game_move(Game *g, int dx);           /* left/right */
bool game_soft_drop(Game *g);              /* down one row */
int game_hard_drop(Game *g);               /* drop to floor, returns cells dropped */
bool game_rotate(Game *g, int dir);        /* +1 clockwise, -1 counter */
bool game_hold(Game *g);                   /* swap with hold slot */
int game_tick_gravity(Game *g);            /* rows the piece moved down */

int drop_interval_ms(int level);           /* speed curve */
bool load_scores(const char *path, int *out, int *count); /* top scores desc */
bool save_score(const char *path, int score, int *rank_out);

#endif /* TETRIS_H */
