#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <libxml/parser.h>
#include <locale.h>

#include "twitter.h"
#include "config.h"
#include "twiauth.h"
#include "twiaction.h"
#include "filter.h"
#include "twiparse.h"
#include "ui.h"

WINDOW *create_newwin(int height, int width, int starty, int startx){
    WINDOW *local_win;

    local_win = newwin(height, width, starty, startx);
    box(local_win, 0 , 0);
    wrefresh(local_win);

    return local_win;
}

void destroy_win(WINDOW *local_win){ 
    box(local_win, ' ', ' '); 
    wborder(local_win, ' ', ' ', ' ',' ',' ',' ',' ',' ');
    wrefresh(local_win);
    delwin(local_win);
}

void filter_display(WINDOW *win, status *p){
    int y,x;
    if(p->filter_count > 0){
        for(int j = 0; j < p->filter_count; j++){
            display_filter *fil = p->filter_list[j];
            if(fil)
                fil->before_filter(win);
            getyx(win,y,x);
            p->x_filter_begin[j] = x;
            p->y_filter_begin[j] = y;
            waddstr(win,p->filtered_text[j]);
            getyx(win,y,x);
            p->x_filter_end[j] = x;
            p->y_filter_end[j] = y;
            if(fil)
                fil->after_filter(win);
        }
    }
    else
        waddstr(win,p->text);
}

int show_status(WINDOW *win,status *p){
    int y,x;
    getyx(win,y,x);
    p->y_min = y;

    init_pair(5, COLOR_GREEN, COLOR_BLACK);
    wattron(win,COLOR_PAIR(5));
    waddstr(win,p->composer.screen_name);
    wattroff(win,COLOR_PAIR(5));
    waddch(win,'\n');

    filter_display(win,p);

    getyx(win,y,x);
    p->y_max = y;

    waddch(win,'\n');
}

status *show_timeline(WINDOW *win, status *p,int height, int width){ 
    char border[width+1];
    for(int i=0;i<width;i++)
        border[i] = '-';
    border[width] = '\0';

    wmove(win,0,0);
    while(p){
        show_status(win,p);
        waddstr(win,border);
        if(p->y_max >= height-3)
            break;
        p = p->next;
    }
    return p;
}

void highlight_status(WINDOW *win, status *s){
    for(int line = s->y_min; line <= s->y_max; ++line){
        wmove(win,line,0);
        wchgat(win,-1,A_REVERSE,0,NULL);
    }
    wrefresh(win);
}

