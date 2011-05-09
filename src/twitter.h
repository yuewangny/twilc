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


#ifndef TWITTER_H
#define TWITTER_H

#include <stdint.h>
#include "filter.h"
#include "config.h"

#define MAX_FILTERS_PER_STATUS 50
typedef struct {
    char *id;
    char *screen_name;
} user;

typedef struct status_str{
    // content of the status
    char *id;
    user *composer;
    user *retweeter;
    char *text;

    // position to show
    int y_min;
    int y_max;

    uint8_t extra_info;

    char *filtered_text[MAX_FILTERS_PER_STATUS];
    display_filter *filter_list[MAX_FILTERS_PER_STATUS];
    int filter_count;
    int x_filter_begin[MAX_FILTERS_PER_STATUS];
    int y_filter_begin[MAX_FILTERS_PER_STATUS];
    int x_filter_end[MAX_FILTERS_PER_STATUS];
    int y_filter_end[MAX_FILTERS_PER_STATUS];

    struct status_str *prev;
    struct status_str *next;
} status;

typedef struct{
    status *head;
    int count;
} statuses;

#define IS_SEPARATED(a)  a & 0x80
#define SET_SEPARATED(a) a |= 0x80
#define UNSET_SEPARATED(a) a &= 0x7F

#define IS_MENTIONED(a)  a & 0x40
#define SET_MENTIONED(a) a |= 0x40

#define IS_RETWEETED(a) a & 0x20
#define SET_RETWEETED(a) a |= 0x20

#define IS_FROMSELF(a) a & 0x10
#define SET_FROMSELF(a) a |= 0x10

#define IS_FAVORITED(a) a & 0x08
#define SET_FAVORITED(a) a |= 0x08
#define UNSET_FAVORITED(a) a &= 0xF7

#define IS_RETWEETEDBYSELF a & 0x04
#define SET_RETWEETEDBYSELF a |= 0x04
#define UNSET_RETWEETEDBYSELF a &= 0xFB

#define TIMELINE_COUNT 1 //0 for home, 1 for mention

statuses *timelines[TIMELINE_COUNT];
status *current_status[TIMELINE_COUNT];
status *current_top_status[TIMELINE_COUNT];
status *current_bottom_status[TIMELINE_COUNT];
status *last_viewed_status[TIMELINE_COUNT];
status *separate_status[TIMELINE_COUNT];

int current_tl_index;

int authorize(clit_config *config);
int update_timeline(int tl_index, status *from_status, status *to_status);
int load_timeline(char *tmpfile, statuses *tl,status *from,status *to);
void filter_status_text(status *s);

user *newuser();
int destroy_user();
status *newstatus();
int init_timelines();
int destroy_timeline(statuses *tl);
int destroy_status(status *s);

#endif
