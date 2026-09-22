#include "tetris.h"

#include <stdio.h>
#include <string.h>

/* --- piece definitions -----------------------------------------------------
 * Rotation states follow SRS: spawn (0), right (1), 2 (2), left (3).
 * Cell offsets are given with y growing downward (board orientation).
 */

const PieceDef PIECES[NUM_PIECES] = {
    /* I — 4x4 box */
    {.cells = {
         {{0, 1}, {1, 1}, {2, 1}, {3, 1}}, /* spawn: row 1 */
         {{2, 0}, {2, 1}, {2, 2}, {2, 3}}, /* right: column 2 */
         {{0, 2}, {1, 2}, {2, 2}, {3, 2}}, /* 2: row 2 */
         {{1, 0}, {1, 1}, {1, 2}, {1, 3}}, /* left: column 1 */
     },
     .kicks = {
         {{0, 0}, {-2, 0}, {1, 0}, {-2, 1}, {1, -2}},    /* 0->1 */
         {{0, 0}, {2, 0}, {-1, 0}, {2, -1}, {-1, 2}},    /* 1->0 */
         {{0, 0}, {-1, 0}, {2, 0}, {-1, 2}, {2, -1}},    /* 1->2 */
         {{0, 0}, {1, 0}, {-2, 0}, {1, -2}, {-2, 1}},    /* 2->1 */
         {{0, 0}, {2, 0}, {-1, 0}, {2, -1}, {-1, 2}},    /* 2->3 */
         {{0, 0}, {-2, 0}, {1, 0}, {-2, 1}, {1, -2}},    /* 3->2 */
         {{0, 0}, {1, 0}, {-2, 0}, {1, -2}, {-2, 1}},    /* 3->0 */
         {{0, 0}, {-1, 0}, {2, 0}, {-1, 2}, {2, -1}},    /* 0->3 */
     }},
    /* J — 3x3 box */
    {.cells = {
         {{0, 0}, {0, 1}, {1, 1}, {2, 1}},
         {{1, 0}, {2, 0}, {1, 1}, {1, 2}},
         {{0, 1}, {1, 1}, {2, 1}, {2, 2}},
         {{1, 0}, {1, 1}, {0, 2}, {1, 2}},
     },
     .kicks = {
         {{0, 0}, {-1, 0}, {-1, -1}, {0, 2}, {-1, 2}},   /* 0->1 (srs y flipped) */
         {{0, 0}, {1, 0}, {1, 1}, {0, -2}, {1, -2}},
         {{0, 0}, {1, 0}, {1, 1}, {0, -2}, {1, -2}},
         {{0, 0}, {-1, 0}, {-1, -1}, {0, 2}, {-1, 2}},
         {{0, 0}, {1, 0}, {1, -1}, {0, 2}, {1, 2}},
         {{0, 0}, {-1, 0}, {-1, 1}, {0, -2}, {-1, -2}},
         {{0, 0}, {-1, 0}, {-1, 1}, {0, -2}, {-1, -2}},
         {{0, 0}, {1, 0}, {1, -1}, {0, 2}, {1, 2}},
     }},
    /* L */
    {.cells = {
         {{2, 0}, {0, 1}, {1, 1}, {2, 1}},
         {{1, 0}, {1, 1}, {1, 2}, {2, 2}},
         {{0, 1}, {1, 1}, {2, 1}, {0, 2}},
         {{0, 0}, {1, 0}, {1, 1}, {1, 2}},
     },
     .kicks = {
         {{0, 0}, {-1, 0}, {-1, -1}, {0, 2}, {-1, 2}},
         {{0, 0}, {1, 0}, {1, 1}, {0, -2}, {1, -2}},
         {{0, 0}, {1, 0}, {1, 1}, {0, -2}, {1, -2}},
         {{0, 0}, {-1, 0}, {-1, -1}, {0, 2}, {-1, 2}},
         {{0, 0}, {1, 0}, {1, -1}, {0, 2}, {1, 2}},
         {{0, 0}, {-1, 0}, {-1, 1}, {0, -2}, {-1, -2}},
         {{0, 0}, {-1, 0}, {-1, 1}, {0, -2}, {-1, -2}},
         {{0, 0}, {1, 0}, {1, -1}, {0, 2}, {1, 2}},
     }},
    /* O — no kicks needed, rotation is identity */
    {.cells = {
         {{1, 0}, {2, 0}, {1, 1}, {2, 1}},
         {{1, 0}, {2, 0}, {1, 1}, {2, 1}},
         {{1, 0}, {2, 0}, {1, 1}, {2, 1}},
         {{1, 0}, {2, 0}, {1, 1}, {2, 1}},
     },
     .kicks = {
         {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}},
         {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}},
         {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}},
         {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}},
         {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}},
         {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}},
         {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}},
         {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}},
     }},
    /* S */
    {.cells = {
         {{1, 0}, {2, 0}, {0, 1}, {1, 1}},
         {{1, 0}, {1, 1}, {2, 1}, {2, 2}},
         {{1, 1}, {2, 1}, {0, 2}, {1, 2}},
         {{0, 0}, {0, 1}, {1, 1}, {1, 2}},
     },
     .kicks = {
         {{0, 0}, {-1, 0}, {-1, -1}, {0, 2}, {-1, 2}},
         {{0, 0}, {1, 0}, {1, 1}, {0, -2}, {1, -2}},
         {{0, 0}, {1, 0}, {1, 1}, {0, -2}, {1, -2}},
         {{0, 0}, {-1, 0}, {-1, -1}, {0, 2}, {-1, 2}},
         {{0, 0}, {1, 0}, {1, -1}, {0, 2}, {1, 2}},
         {{0, 0}, {-1, 0}, {-1, 1}, {0, -2}, {-1, -2}},
         {{0, 0}, {-1, 0}, {-1, 1}, {0, -2}, {-1, -2}},
         {{0, 0}, {1, 0}, {1, -1}, {0, 2}, {1, 2}},
     }},
    /* T */
    {.cells = {
         {{1, 0}, {0, 1}, {1, 1}, {2, 1}},
         {{1, 0}, {1, 1}, {2, 1}, {1, 2}},
         {{0, 1}, {1, 1}, {2, 1}, {1, 2}},
         {{1, 0}, {0, 1}, {1, 1}, {1, 2}},
     },
     .kicks = {
         {{0, 0}, {-1, 0}, {-1, -1}, {0, 2}, {-1, 2}},
         {{0, 0}, {1, 0}, {1, 1}, {0, -2}, {1, -2}},
         {{0, 0}, {1, 0}, {1, 1}, {0, -2}, {1, -2}},
         {{0, 0}, {-1, 0}, {-1, -1}, {0, 2}, {-1, 2}},
         {{0, 0}, {1, 0}, {1, -1}, {0, 2}, {1, 2}},
         {{0, 0}, {-1, 0}, {-1, 1}, {0, -2}, {-1, -2}},
         {{0, 0}, {-1, 0}, {-1, 1}, {0, -2}, {-1, -2}},
         {{0, 0}, {1, 0}, {1, -1}, {0, 2}, {1, 2}},
     }},
    /* Z */
    {.cells = {
         {{0, 0}, {1, 0}, {1, 1}, {2, 1}},
         {{2, 0}, {1, 1}, {2, 1}, {1, 2}},
         {{0, 1}, {1, 1}, {1, 2}, {2, 2}},
         {{1, 0}, {0, 1}, {1, 1}, {0, 2}},
     },
     .kicks = {
         {{0, 0}, {-1, 0}, {-1, -1}, {0, 2}, {-1, 2}},
         {{0, 0}, {1, 0}, {1, 1}, {0, -2}, {1, -2}},
         {{0, 0}, {1, 0}, {1, 1}, {0, -2}, {1, -2}},
         {{0, 0}, {-1, 0}, {-1, -1}, {0, 2}, {-1, 2}},
         {{0, 0}, {1, 0}, {1, -1}, {0, 2}, {1, 2}},
         {{0, 0}, {-1, 0}, {-1, 1}, {0, -2}, {-1, -2}},
         {{0, 0}, {-1, 0}, {-1, 1}, {0, -2}, {-1, -2}},
         {{0, 0}, {1, 0}, {1, -1}, {0, 2}, {1, 2}},
     }},
};

/* --- 7-bag randomizer ------------------------------------------------------ */

static uint64_t next_rng(uint64_t *state) {
    /* xorshift64* */
    uint64_t x = *state;
    x ^= x >> 12;
    x ^= x << 25;
    x ^= x >> 27;
    *state = x;
    return x * 0x2545F4914F6CDD1DULL;
}

static void refill_bag(Game *g, int offset) {
    /* fill offset..offset+NUM_PIECES-1 with a shuffled permutation */
    int bag[NUM_PIECES];
    for (int i = 0; i < NUM_PIECES; i++) bag[i] = i;
    for (int i = NUM_PIECES - 1; i > 0; i--) {
        int j = (int)(next_rng(&g->rngState) % (uint64_t)(i + 1));
        int tmp = bag[i];
        bag[i] = bag[j];
        bag[j] = tmp;
    }
    for (int i = 0; i < NUM_PIECES; i++) {
        g->nextQueue[offset + i] = bag[i];
    }
}

/* --- board helpers ---------------------------------------------------------- */

static bool cell_free(const Game *g, int x, int y) {
    if (x < 0 || x >= BOARD_W || y >= BOARD_H) return false;
    if (y < 0) return true; /* above the board is free (spawn area) */
    return g->board[y][x] == 0;
}

static bool placement_fits(const Game *g, int kind, int rot, int x, int y) {
    const PieceDef *def = &PIECES[kind];
    for (int i = 0; i < 4; i++) {
        int cx = x + def->cells[rot][i][0];
        int cy = y + def->cells[rot][i][1];
        if (!cell_free(g, cx, cy)) return false;
    }
    return true;
}

static int clear_lines(Game *g) {
    int cleared = 0;
    for (int row = BOARD_H - 1; row >= 0; row--) {
        bool full = true;
        for (int x = 0; x < BOARD_W; x++) {
            if (g->board[row][x] == 0) {
                full = false;
                break;
            }
        }
        if (!full) continue;
        cleared++;
        for (int r = row; r > 0; r--) {
            memcpy(g->board[r], g->board[r - 1], sizeof(g->board[0]));
        }
        memset(g->board[0], 0, sizeof(g->board[0]));
        row++; /* re-check the same row after the shift */
    }
    return cleared;
}

static void spawn_piece(Game *g) {
    g->kind = g->nextQueue[g->bagPos++];
    if (g->bagPos == NUM_PIECES) {
        /* shift next bag to the front, refill the back half */
        for (int i = 0; i < NUM_PIECES; i++) {
            g->nextQueue[i] = g->nextQueue[NUM_PIECES + i];
        }
        g->bagPos = 0;
        refill_bag(g, NUM_PIECES);
    }
    g->rot = 0;
    g->x = 3;
    g->y = -1;
    g->holdUsed = false;
    if (!placement_fits(g, g->kind, g->rot, g->x, g->y)) {
        /* try one row up, then give up: game over */
        g->y = -2;
        if (!placement_fits(g, g->kind, g->rot, g->x, g->y)) {
            g->gameOver = true;
        }
    }
}

void game_init(Game *g, uint64_t seed) {
    memset(g, 0, sizeof(*g));
    g->rngState = seed ? seed : 1;
    g->level = 1;
    g->holdPiece = -1;
    refill_bag(g, 0);
    refill_bag(g, NUM_PIECES);
    g->bagPos = 0;
    spawn_piece(g);
}

/* --- actions ----------------------------------------------------------------- */

bool game_move(Game *g, int dx) {
    if (g->gameOver) return false;
    if (placement_fits(g, g->kind, g->rot, g->x + dx, g->y)) {
        g->x += dx;
        return true;
    }
    return false;
}

bool game_soft_drop(Game *g) {
    if (g->gameOver) return false;
    if (placement_fits(g, g->kind, g->rot, g->x, g->y + 1)) {
        g->y++;
        g->score += 1;
        return true;
    }
    /* lock the piece */
    const PieceDef *def = &PIECES[g->kind];
    for (int i = 0; i < 4; i++) {
        int cx = g->x + def->cells[g->rot][i][0];
        int cy = g->y + def->cells[g->rot][i][1];
        if (cy >= 0) g->board[cy][cx] = g->kind + 1;
    }
    int cleared = clear_lines(g);
    if (cleared > 0) {
        static const int base[5] = {0, 100, 300, 500, 800};
        g->score += base[cleared] * g->level;
        g->lines += cleared;
        g->level = 1 + g->lines / 10;
    }
    spawn_piece(g);
    return true;
}

/* one gravity step; locks the piece (without the soft-drop score bonus)
 * when it can no longer descend, returns false in that case */
static bool game_soft_drop_in_place(Game *g);

int game_hard_drop(Game *g) {
    int dropped = 0;
    while (game_soft_drop_in_place(g)) dropped++;
    g->score += dropped * 2;
    return dropped;
}

static bool game_soft_drop_in_place(Game *g) {
    if (placement_fits(g, g->kind, g->rot, g->x, g->y + 1)) {
        g->y++;
        return true;
    }
    const PieceDef *def = &PIECES[g->kind];
    for (int i = 0; i < 4; i++) {
        int cx = g->x + def->cells[g->rot][i][0];
        int cy = g->y + def->cells[g->rot][i][1];
        if (cy >= 0) g->board[cy][cx] = g->kind + 1;
    }
    int cleared = clear_lines(g);
    if (cleared > 0) {
        static const int base[5] = {0, 100, 300, 500, 800};
        g->score += base[cleared] * g->level;
        g->lines += cleared;
        g->level = 1 + g->lines / 10;
    }
    spawn_piece(g);
    return false; /* locked */
}

bool game_rotate(Game *g, int dir) {
    if (g->gameOver) return false;
    int from = g->rot;
    int to = (from + (dir > 0 ? 1 : 3)) % 4;
    /* kick-table order: [0->1, 1->0, 1->2, 2->1, 2->3, 3->2, 3->0, 0->3] */
    int tableIndex = dir > 0 ? from * 2 : ((from + 3) % 4) * 2 + 1;
    if (g->kind == O) {
        g->rot = to;
        return true;
    }
    const PieceDef *def = &PIECES[g->kind];
    for (int t = 0; t < 5; t++) {
        int dx = def->kicks[tableIndex][t][0];
        int dy = def->kicks[tableIndex][t][1];
        if (placement_fits(g, g->kind, to, g->x + dx, g->y + dy)) {
            g->x += dx;
            g->y += dy;
            g->rot = to;
            return true;
        }
    }
    return false;
}

bool game_hold(Game *g) {
    if (g->gameOver || g->holdUsed) return false;
    int current = g->kind;
    if (g->holdPiece == -1) {
        g->holdPiece = current;
        spawn_piece(g);
    } else {
        int swap = g->holdPiece;
        g->holdPiece = current;
        g->kind = swap;
        g->rot = 0;
        g->x = 3;
        g->y = -1;
        if (!placement_fits(g, g->kind, g->rot, g->x, g->y)) {
            g->gameOver = true;
            return false;
        }
    }
    g->holdUsed = true;
    return true;
}

int game_tick_gravity(Game *g) {
    game_soft_drop(g);
    return 0;
}

int drop_interval_ms(int level) {
    int ms = 900 - (level - 1) * 80;
    if (ms < 60) ms = 60;
    return ms;
}

/* --- local scoreboard --------------------------------------------------------- */

bool load_scores(const char *path, int *out, int *count) {
    FILE *fh = fopen(path, "r");
    *count = 0;
    if (!fh) return false;
    int value;
    while (*count < 10 && fscanf(fh, "%d", &value) == 1) {
        out[(*count)++] = value;
    }
    fclose(fh);
    return true;
}

bool save_score(const char *path, int score, int *rank_out) {
    int scores[10];
    int count = 0;
    load_scores(path, scores, &count);
    int pos = count;
    for (int i = 0; i < count; i++) {
        if (score > scores[i]) {
            pos = i;
            break;
        }
    }
    if (pos >= 10) {
        if (rank_out) *rank_out = -1;
        return true; /* not a top-10 score; nothing stored */
    }
    for (int i = (count < 10 ? count : 9); i > pos; i--) {
        scores[i] = scores[i - 1];
    }
    scores[pos] = score;
    if (count < 10) count++;
    FILE *fh = fopen(path, "w");
    if (!fh) return false;
    for (int i = 0; i < count; i++) fprintf(fh, "%d\n", scores[i]);
    fclose(fh);
    if (rank_out) *rank_out = pos;
    return true;
}
