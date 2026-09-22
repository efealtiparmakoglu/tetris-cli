/* tetris CLI: raw-terminal game loop (POSIX) plus score table viewing.
 * The game logic lives in tetris.c and is fully unit-tested. */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#include "tetris.h"

#define SCORES_FILE "tetris-scores.txt"

static struct termios g_origTermios;
static int g_rawMode = 0;

static void enable_raw_mode(void) {
    struct termios raw;
    if (tcgetattr(STDIN_FILENO, &g_origTermios) != 0) return;
    raw = g_origTermios;
    raw.c_lflag &= ~(ICANON | ECHO);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
    g_rawMode = 1;
}

static void disable_raw_mode(void) {
    if (g_rawMode) tcsetattr(STDIN_FILENO, TCSAFLUSH, &g_origTermios);
}

static void render_frame(const int visible[BOARD_H][BOARD_W], const Game *g);

static long ms_since(const struct timespec *start) {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return (now.tv_sec - start->tv_sec) * 1000L + (now.tv_nsec - start->tv_nsec) / 1000000L;
}

/* overlay the active piece onto a copy for rendering */
static void draw_piece(Game *g, int visible[BOARD_H][BOARD_W]) {
    memcpy(visible, g->board, sizeof(g->board));
    if (g->gameOver) return;
    const PieceDef *def = &PIECES[g->kind];
    for (int i = 0; i < 4; i++) {
        int cx = g->x + def->cells[g->rot][i][0];
        int cy = g->y + def->cells[g->rot][i][1];
        if (cy >= 0 && cy < BOARD_H && cx >= 0 && cx < BOARD_W) {
            visible[cy][cx] = g->kind + 1;
        }
    }
}

static void run_game(uint64_t seed, const char *scoresPath) {
    Game game;
    game_init(&game, seed);

    enable_raw_mode();
    atexit(disable_raw_mode);

    struct timespec last;
    clock_gettime(CLOCK_MONOTONIC, &last);

    static int visible[BOARD_H][BOARD_W];
    for (;;) {
        draw_piece(&game, visible);
        render_frame(visible, &game);

        /* input: drain every available key */
        char key;
        while (read(STDIN_FILENO, &key, 1) == 1) {
            switch (key) {
                case 'h': game_move(&game, -1); break;
                case 'l': game_move(&game, 1); break;
                case 'j': game_soft_drop(&game); break;
                case 'k': game_rotate(&game, 1); break;
                case 'z': game_rotate(&game, -1); break;
                case ' ': game_hard_drop(&game); break;
                case 'c': game_hold(&game); break;
                case 'q': goto done;
                default: break;
            }
        }

        if (!game.gameOver) {
            long elapsed = ms_since(&last);
            long interval = drop_interval_ms(game.level);
            if (elapsed >= interval) {
                game_soft_drop(&game);
                clock_gettime(CLOCK_MONOTONIC, &last);
            }
        } else {
            int rank = -1;
            save_score(scoresPath, game.score, &rank);
            render_frame(visible, &game);
            printf("final score %d", game.score);
            if (rank >= 0) printf(" (local rank #%d)", rank + 1);
            printf(" — saved to %s\n", scoresPath);
            break;
        }
        struct timespec sleep = {0, 20 * 1000000L};
        nanosleep(&sleep, NULL);
    }
done:
    printf("bye\n");
}

/* render taking the overlay into account (kept separate for clarity) */
static void render_frame(const int visible[BOARD_H][BOARD_W], const Game *g) {
    char buf[BOARD_H * BOARD_W * 4 + 512];
    int pos = 0;
    pos += snprintf(buf + pos, sizeof(buf) - (size_t)pos,
                    "\033[H\033[2Jscore %d | lines %d | level %d\n",
                    g->score, g->lines, g->level);
    for (int y = 0; y < BOARD_H; y++) {
        pos += snprintf(buf + pos, sizeof(buf) - (size_t)pos, "|");
        for (int x = 0; x < BOARD_W; x++) {
            pos += snprintf(buf + pos, sizeof(buf) - (size_t)pos,
                            visible[y][x] ? "[]" : " .");
        }
        pos += snprintf(buf + pos, sizeof(buf) - (size_t)pos, "|\n");
    }
    if (g->gameOver) {
        pos += snprintf(buf + pos, sizeof(buf) - (size_t)pos, "GAME OVER\n");
    }
    fwrite(buf, 1, (size_t)pos, stdout);
    fflush(stdout);
}

static void show_scores(const char *path) {
    int scores[10];
    int count = 0;
    printf("local high scores (%s):\n", path);
    if (!load_scores(path, scores, &count) || count == 0) {
        printf("  (empty)\n");
        return;
    }
    for (int i = 0; i < count; i++) {
        printf("  %2d. %d\n", i + 1, scores[i]);
    }
}

int main(int argc, char **argv) {
    uint64_t seed = (uint64_t)time(NULL);
    const char *scoresPath = SCORES_FILE;

    if (argc >= 2 && strcmp(argv[1], "score") == 0) {
        show_scores(argc >= 3 ? argv[2] : scoresPath);
        return 0;
    }
    if (argc >= 2 && strcmp(argv[1], "play") == 0) {
        if (argc >= 4 && strcmp(argv[2], "-s") == 0) {
            seed = strtoull(argv[3], NULL, 10);
        }
        if (argc >= 4 && strcmp(argv[2], "--scores") == 0) scoresPath = argv[3];
        run_game(seed, scoresPath);
        return 0;
    }
    fprintf(stderr,
            "tetris-cli v1.0.0\n"
            "USAGE: tetris-cli play [-s seed] [--scores file] | score [file]\n"
            "keys: h/l move, j soft drop, k/z rotate, space hard drop, c hold, q quit\n");
    return 2;
}
