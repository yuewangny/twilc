/**
 * Twilc is an NCURSES based CLI twitter client for Linux.
 * Copyright (C) 2011 Margaret Wang (pipituliuliu@gmail.com)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 **/


#ifndef UI_H
#define UI_H

#include <ncurses.h>
#include <stdint.h>

#include "twitter.h"

int show_status(WINDOW *win, struct status_node *sn);

struct status_node *show_timeline(WINDOW *win, struct status_node *sn, int height, int width);

int highlight_status(WINDOW *win, struct status_node *sn);

int init_ui();
int destroy_ui();
int notify_error_state();
int notify_state_change(const char *);
int refresh_status_height(WINDOW *,struct status_node *,struct status_node *);

WINDOW *title_win;
WINDOW *tl_win;
WINDOW *state_win;

#define STATE_NORMAL 0
#define STATE_REACHED_TOP 1
#define STATE_REACHED_BOTTOM 2
#define STATE_RETRIEVING_UPDATES 3 
#define STATE_TL_UPDATED 4
#define STATE_RETRIEVE_FAILED 5 
#define STATE_LOADING_UPDATES 6 
#define STATE_NO_LAST_VIEWED 7

#define NR_STATES 8 

const char *states[NR_STATES];

#endif
