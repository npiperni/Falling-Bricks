#pragma once

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720

#define FPS 60
#define FRAME_TARGET_TIME (1000 / FPS)

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#define LABEL_DEFAULT_FONT_SIZE 40
#define LABEL_DEFAULT_SMALL_FONT_SIZE 30

#define BOARD_WIDTH 10
#define BOARD_HEIGHT 20
#define CELL_SIZE 32

#define BLITZ_TIME 120000 // 2 minutes

#define ROW_LABEL_DISPLAY_DURATION 1800
#define COUNTDOWN_DISPLAY_DURATION 1000 // 1 second
#define LEVEL_UP_LABEL_DISPLAY_DURATION 2500 // 2.5 seconds

#define BASE_DROP_SPEED 1000 // 1 second
#define BASE_LINES_PER_LEVEL 10
