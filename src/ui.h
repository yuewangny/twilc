#ifndef UI_H
#define UI_H

#include <ncurses.h>

#include "twitter.h"

WINDOW *create_newwin(int height, int width, int starty, int startx);

void destroy_win(WINDOW *local_win);

int show_status(WINDOW *win, status *s);

status *show_timeline(WINDOW *win, status *s, int height, int width);

int highlight_status(WINDOW *win, status *s);

#endif
