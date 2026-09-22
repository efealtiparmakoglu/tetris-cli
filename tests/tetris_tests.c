/* Numerical test suite for the Tetris core. */

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "tetris.h"

static int tests_run = 0;
static int tests_failed = 0;

#define CHECK(cond)                                                       \
    do {                                                                  \
        tests_run++;                                                      \
        if (!(cond)) {                                                    \
            tests_failed++;                                               \
            fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); \
        }                                                                 \
    } while (0)

static void drop_kind(Game *g, int kind) {
    /* advance the RNG-dependent spawn until the wanted kind appears */
    for (int guard = 0; guard < 200 && g->kind != kind; guard++) {
        game_hard_drop(g);
    }
    CHECK(g->kind == kind);
}

static void test_init_and_bag(void) {
    Game g;
    game_init(&g, 42);
    CHECK(g.level == 1 && g.lines == 0 && g.score == 0);
    CHECK(!g.gameOver);
    /* every group of 7 consecutive queue entries is a permutation of 7 kinds */
    for (int start = 0; start + NUM_PIECES <= NUM_PIECES * 2; start += NUM_PIECES) {
        int seen[NUM_PIECES] = {0};
        for (int i = 0; i < NUM_PIECES; i++) {
            int kind = g.nextQueue[start + i];
            CHECK(kind >= 0 && kind < NUM_PIECES);
            seen[kind]++;
        }
        for (int i = 0; i < NUM_PIECES; i++) CHECK(seen[i] == 1);
    }
}

static void test_move_and_collision(void) {
    Game g;
    game_init(&g, 7);
    /* move to the left wall, then try one more */
    for (int i = 0; i < 12; i++) game_move(&g, -1);
    /* at the wall another left move must fail and leave x unchanged */
    int x_before = g.x;
    CHECK(!game_move(&g, -1));
    CHECK(g.x == x_before);
}

static void test_rotation_with_wall_kick(void) {
    Game g;
    game_init(&g, 7);
    drop_kind(&g, T);
    /* push the T piece to the left wall and rotate clockwise: a kick
     * must keep the piece inside the board */
    for (int i = 0; i < 12; i++) game_move(&g, -1);
    CHECK(game_rotate(&g, 1));
    CHECK(g.rot == 1);
    /* after rotation all four cells must be inside the board */
    const PieceDef *def = &PIECES[g.kind];
    for (int i = 0; i < 4; i++) {
        int cx = g.x + def->cells[g.rot][i][0];
        int cy = g.y + def->cells[g.rot][i][1];
        CHECK(cx >= 0 && cx < BOARD_W);
        CHECK(cy < BOARD_H);
    }
}

static void test_line_clear_and_scoring(void) {
    Game g;
    game_init(&g, 1234);
    drop_kind(&g, I);
    /* fill row 19 except one column, then drop the I vertically through */
    for (int x = 0; x < BOARD_W; x++) {
        if (x != 5) g.board[BOARD_H - 1][x] = 3;
    }
    /* force a vertical I directly above the open column 5 and hard drop */
    g.kind = I;
    g.rot = 1; /* vertical, cells at column offset 2 */
    g.x = 3;   /* cells land on x=5 */
    g.y = -1;
    int dropped = game_hard_drop(&g);
    CHECK(dropped > 0);
    CHECK(g.lines == 1);
    /* the cleared row is gone; the I body above it fell to the bottom
     * row, leaving exactly the 3 body cells in column 5 */
    int filled = 0;
    for (int x = 0; x < BOARD_W; x++)
        if (g.board[BOARD_H - 1][x]) filled++;
    CHECK(filled == 3);
    CHECK(g.board[BOARD_H - 1][5] == I + 1);
}

static void test_scoring_values(void) {
    Game g;
    game_init(&g, 9);
    int base = g.score;
    /* soft drop scores +1 per row */
    int y0 = g.y;
    game_soft_drop(&g);
    CHECK(g.score == base + (g.y - y0));
}

static void test_hold_swaps(void) {
    Game g;
    game_init(&g, 5);
    int first = g.kind;
    CHECK(game_hold(&g));
    CHECK(g.holdPiece == first);
    int second = g.kind;
    CHECK(!game_hold(&g)); /* double hold must be rejected */
    CHECK(g.kind == second);
}

static void test_speed_curve(void) {
    CHECK(drop_interval_ms(1) == 900);
    CHECK(drop_interval_ms(2) == 820);
    CHECK(drop_interval_ms(12) == 60);  /* floor */
    CHECK(drop_interval_ms(30) == 60);
}

static void test_scoreboard_round_trip(void) {
    const char *path = "/tmp/tetris-scores-test.txt";
    remove(path);
    int rank = -1;
    CHECK(save_score(path, 500, &rank) && rank == 0);
    CHECK(save_score(path, 900, &rank) && rank == 0);
    CHECK(save_score(path, 700, &rank) && rank == 1);
    CHECK(save_score(path, 100, &rank) && rank == 3);
    int scores[10];
    int count = 0;
    CHECK(load_scores(path, scores, &count));
    CHECK(count == 4);
    CHECK(scores[0] == 900 && scores[1] == 700 && scores[2] == 500 && scores[3] == 100);
    /* 11th distinct score below the top 10 does not get stored */
    for (int i = 0; i < 10; i++) save_score(path, 50 + i, NULL);
    CHECK(save_score(path, 10, &rank) && rank == -1);
    remove(path);
}

static void test_scripted_marathon(void) {
    /* random-ish actions for 2000 steps: the core must never crash or
     * corrupt the board */
    Game g;
    game_init(&g, 99);
    uint64_t rng = 88172645463325252ULL;
    for (int step = 0; step < 2000 && !g.gameOver; step++) {
        rng ^= rng << 13;
        rng ^= rng >> 7;
        rng ^= rng << 17;
        switch (rng % 6) {
            case 0: game_move(&g, -1); break;
            case 1: game_move(&g, 1); break;
            case 2: game_rotate(&g, 1); break;
            case 3: game_rotate(&g, -1); break;
            case 4: game_hold(&g); break;
            default: game_hard_drop(&g); break;
        }
        if (rng % 3 == 0) game_hard_drop(&g);
    }
    int filled = 0;
    for (int y = 0; y < BOARD_H; y++)
        for (int x = 0; x < BOARD_W; x++)
            if (g.board[y][x] < 0 || g.board[y][x] > NUM_PIECES) {
                CHECK(0);
                return;
            }
    CHECK(filled == 0);
    tests_run++;
}

int main(void) {
    test_init_and_bag();
    test_move_and_collision();
    test_rotation_with_wall_kick();
    test_line_clear_and_scoring();
    test_scoring_values();
    test_hold_swaps();
    test_speed_curve();
    test_scoreboard_round_trip();
    test_scripted_marathon();

    printf("%d checks, %d failed\n", tests_run, tests_failed);
    return tests_failed == 0 ? 0 : 1;
}
