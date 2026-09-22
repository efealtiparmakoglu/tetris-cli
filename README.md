# tetris-cli

Terminal Tetris in C11 — **real SRS rotation with wall kicks**, 7-bag
randomizer, hold slot, guideline scoring, a speed curve and a local
high-score file. No asset files, no dependencies: just ANSI escapes
and raw terminal mode.

```text
$ tetris-cli play -s 42
score 1200 | lines 14 | level 2
|[][] . . [][] . [][] . |
|. . [][] [][] . [][][][]|
...
```

## The pieces

- **SRS rotation** — every piece carries its four rotation states plus
  the standard five-entry wall-kick tables (separate tables for I and
  JLSTZ, O is a no-op), converted to board orientation (y down). The
  tests rotate a T piece against the wall and prove kicks keep every
  cell on the board.
- **7-bag randomizer** — the queue is a shuffled permutation of the
  seven kinds refilled two bags ahead; every 7-window is a full
  permutation (asserted).
- **Scoring** — 100/300/500/800 × level for single..tetris, +1 per soft
  drop cell, +2 per hard drop cell. Level climbs every 10 lines.
- **Speed curve** — `drop_interval_ms(level)` = 900 − 80·(level−1),
  floored at 60 ms.
- **Local scoreboard** — `tetris-scores.txt`, top-10 insert with rank
  reporting; lower scores above capacity are dropped.

## Keys

`h`/`l` move · `j` soft drop · `k`/`z` rotate CW/CCW · `space` hard
drop · `c` hold · `q` quit

## CLI

```
tetris-cli play [-s seed] [--scores file]
tetris-cli score [file]
```

POSIX terminals only (raw mode via termios). The core lives in
`tetris.c` with zero I/O, which is what the tests exercise.

## Development

```console
$ make test    # 67 checks: SRS kicks, bag permutations, line clears
               # with correct fall-down, scoring, marathon fuzz, scoreboard
```

## License

MIT — see [LICENSE](LICENSE).
