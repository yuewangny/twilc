#include <stdio.h>
#include <ncurses.h>
#include <stdlib.h>

#include "filter.h"

void before_mention(WINDOW *win){
    init_pair(1, COLOR_YELLOW, COLOR_BLACK);
    wattron(win,COLOR_PAIR(1));
}

void after_mention(WINDOW *win){
    wattroff(win, COLOR_PAIR(1));
}

char *get_mention_end(char *begin){
    char *p = begin + 1;
    if((*begin) != '@')
        return 0;

    int x = *p;
    while(x != '\0' && ((x>='a' && x<='z')||(x>='A' && x<='Z') || (x >= '0' && x <= '9')|| x=='_'))
        x = *(++p);
    return p;
}

char *get_rt_end(char *begin){
    if(*begin == 'R' && *(begin + 1) == 'T')
        return begin + 2;
    else
        return NULL;
}

void before_rt(WINDOW *win){
    waddstr(win,"\n");
    init_pair(2, COLOR_RED, COLOR_BLACK);
    wattron(win,COLOR_PAIR(2));
}

void after_rt(WINDOW *win){
    wattroff(win, COLOR_PAIR(2));
}

char *get_url_end(char *begin){
    char *p;
    for(p = begin + 7;(*p) != '\0';++p){
        int x = *p;
        if((x>='a' && x<='z')||(x>='A' && x<='Z') || (x >= '0' && x <= '9')|| x=='_' || x=='-' || x=='=' || x=='$' || x=='.' || x==',' || x=='*' || x=='+' || x=='(' || x==')' || x== '\'' || x=='/')
            continue;
        else
            return p;
    }
    return p;
}

void before_url(WINDOW *win){
    init_pair(3, COLOR_BLUE, COLOR_BLACK);
    wattron(win,COLOR_PAIR(3));
}

void after_url(WINDOW *win){
    wattroff(win,COLOR_PAIR(3));
}

void create_filters(){
    struct filter *mention_filter = malloc(sizeof(struct filter));
    mention_filter->pattern = "@";
    mention_filter->get_pattern_end = get_mention_end;
    mention_filter->before_filter = before_mention;
    mention_filter->after_filter = after_mention;
    filters[0] = mention_filter;

    struct filter *rt_filter = malloc(sizeof(struct filter));
    rt_filter->pattern = "RT";
    rt_filter->get_pattern_end = get_rt_end;
    rt_filter->before_filter = before_rt;
    rt_filter->after_filter = after_rt;
    filters[1] = rt_filter;

    struct filter *url_filter = malloc(sizeof(struct filter));
    url_filter->pattern = "http://";
    url_filter->get_pattern_end = get_url_end;
    url_filter->before_filter = before_url;
    url_filter->after_filter = after_url;
    filters[2] = url_filter;
}

void destroy_filters(){
    for(int i=0;i<FILTER_NUM;i++)
        free(filters[i]);
}