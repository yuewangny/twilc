#ifndef FILTER_H
#define FILTER_H

#define FILTER_NUM 4 

#include <ncurses.h>

typedef struct filter{
    char *pattern;
    char *(*get_pattern_end)(char *);
    void (*before_filter)(WINDOW *);
    void (*after_filter)(WINDOW *);
} display_filter;

display_filter *filters[FILTER_NUM];

void create_filters();
void destroy_filters();

#endif
