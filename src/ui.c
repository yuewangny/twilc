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

char *titles[3] = {"Home","Mention","Profile"};
const char *states[NR_STATES] = {"",
          "You've reached the top.",
          "You've reached the bottom.",
          "Retrieving timeline updates...",
          "Timeline updated.",
          "Failed to retrieve updates.",
          "Loading timeline updates...",
          "No last viewed status."};

int draw_border(WINDOW *win, char c){
    int height,width;
    getmaxyx(win,height,width);

    char border[width+1];
    for(int i=0;i<width;i++)
        border[i] = c;
    border[width] = '\0';
    waddstr(win,border);
}

int init_tl_win(){
    wmove(tl_win,0,0);
    int height,width;
    getmaxyx(tl_win,height,width);
    current_bottom_status[current_tl_index] = show_timeline(tl_win,current_status[current_tl_index],height,width);
    wrefresh(tl_win);
    highlight_status(tl_win,current_status[current_tl_index]);
    return 0;
}

int init_title_win(){
    wmove(title_win,0,0);
    current_tl_index = 0;
    char *title = titles[current_tl_index];

    int height,width;
    getmaxyx(title_win,height,width);
    init_pair(6, COLOR_BLACK, COLOR_YELLOW);
    wattron(title_win,COLOR_PAIR(6));
//    wmove(title_win,0,(width-strlen(title))/2-1);
    waddstr(title_win, titles[current_tl_index]);
    waddch(title_win,'\n');
    for(int i=0;i<width;i++)
        waddch(title_win,' ');
    wattroff(title_win,COLOR_PAIR(6));
    wrefresh(title_win);
    return 0;
}

int init_state_win(int which_state){
    wmove(state_win,0,0);

    int height,width;
    getmaxyx(title_win,height,width);
    init_pair(6, COLOR_BLACK, COLOR_YELLOW);
    wattron(state_win,COLOR_PAIR(6));
    for(int i=0;i<width;i++)
        waddch(state_win,'-');
    wattroff(state_win,COLOR_PAIR(6));
    waddstr(state_win, states[which_state]);
    wrefresh(state_win);
    return 0;
}

int notify_state_change(const char *state_str){
    wmove(state_win,1,0);
    waddstr(state_win, state_str);
    wclrtoeol(state_win);
    wrefresh(state_win);
    return 0;
}

int init_ui(){
    int height,width;
    getmaxyx(stdscr,height,width);
    title_win = newwin(2,width,0,0);
    tl_win = newwin(height-4,width,2,0);
    state_win = newwin(2,width,height-2,0);

    if(!title_win || !tl_win || !state_win)
        return -1;

    init_title_win();
    init_tl_win();
    init_state_win(STATE_NORMAL);
}

int destroy_ui(){
    if(title_win)
        delwin(title_win);
    if(tl_win)
        delwin(tl_win);
    if(state_win)
        delwin(state_win);
}

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
    if(!p)
        return -1;
    init_pair(5, COLOR_GREEN, COLOR_BLACK);
    wattron(win,COLOR_PAIR(5));
    waddstr(win,p->composer->screen_name);
    wattroff(win,COLOR_PAIR(5));
    waddch(win,'\n');

    filter_display(win,p);
    int y,x;
    getyx(win,y,x);
    p->y_max = y;
    waddch(win,'\n');

    return 0;
}

status *show_timeline(WINDOW *win, status *p,int height, int width){ 
    if(!p)
        return 0;

    int y,x;
    wclear(win);
    wmove(win,0,0);
    status *prev = NULL;
    while(p){
        getyx(win,y,x);
        p->y_min = y;
        show_status(win,p);

        if(p->y_max >= height-3)
            break;
        if(IS_SEPARATED(p->extra_info))
            draw_border(win,'~');
        else
            draw_border(win,'-');
        prev = p;
        p = p->next;
    }
    wrefresh(win);
    if(p)
        return p;
    else
        return prev;
}

int highlight_status(WINDOW *win, status *s){
    if(!s)
        return -1;
    for(int line = s->y_min; line <= s->y_max; ++line){
        wmove(win,line,0);
        wchgat(win,-1,A_REVERSE,0,NULL);
    }
    wrefresh(win);

    return 0;
}

