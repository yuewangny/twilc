#ifndef TWITTER_H
#define TWITTER_H

#include <stdint.h>
#include "filter.h"

#define MAX_FILTERS_PER_STATUS 20
typedef struct {
    char *id;
    char *screen_name;
} user;

typedef struct status_str{
    // content of the status
    char *id;
    user composer;
    char *text;

    // position to show
    int y_min;
    int y_max;

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

#define TIMELINE_COUNT 2 //0 for home, 1 for mention

statuses *timelines[TIMELINE_COUNT];
status *current_status[TIMELINE_COUNT];
status *current_top_status[TIMELINE_COUNT];
status *current_bottom_status[TIMELINE_COUNT];

int current_tl_index;

#endif
