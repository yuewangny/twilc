#ifndef UI_H
#define UI_H

#include <ncurses.h>

#include "twitter.h"

WINDOW *create_newwin(int height, int width, int starty, int startx);

void destroy_win(WINDOW *local_win);

int show_status(WINDOW *win, status *s);

status *show_timeline(WINDOW *win, status *s, int height, int width);

int highlight_status(WINDOW *win, status *s);

int init_ui();
int destroy_ui();
int notify_state_change(int);

WINDOW *title_win;
WINDOW *tl_win;
WINDOW *state_win;

#define STATE_NORMAL 0
#define STATE_REACHED_TOP 1
#define STATE_REACHED_BOTTOM 2
#define STATE_TL_UPDATED 3

#endif
