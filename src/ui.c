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

#define _XOPEN_SOURCE

#include <wchar.h>
#include <curses.h>
#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <libxml/parser.h>
#include <locale.h>
#include <pthread.h>

#include "twitter.h"
#include "config.h"
#include "twiauth.h"
#include "twiaction.h"
#include "entity.h"
#include "twiparse.h"
#include "ui.h"
#include "twierror.h"

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
    timelines[current_tl_index]->current_bottom = show_timeline(tl_win,timelines[current_tl_index]->current,height,width);
    wrefresh(tl_win);
    highlight_status(tl_win,timelines[current_tl_index]->current);
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

int notify_error_state(){
    int errnum;
    pthread_mutex_lock(&error_mutex);
    errnum = error_no;
    pthread_mutex_unlock(&error_mutex);
    notify_state_change(get_error_string(errnum));
}

int notify_state_change(const char *state_str){
    wmove(state_win,1,0);
    waddstr(state_win, state_str);
    wclrtoeol(state_win);
    wrefresh(state_win);
    return 0;
}

int notify_timeline_updates(int tl_index, int updates_count){
    wmove(title_win,0,0);
    char *title = titles[current_tl_index];

    int height,width;
    getmaxyx(title_win,height,width);

    init_pair(6, COLOR_BLACK, COLOR_YELLOW);
    wattron(title_win,COLOR_PAIR(6));
    //    wmove(title_win,0,(width-strlen(title))/2-1);
    waddstr(title_win, titles[current_tl_index]);
    if(updates_count > 0){
        char countstr[10];
        sprintf(countstr,"(%d)",updates_count);
        waddstr(title_win, countstr);
    }
    wattroff(title_win,COLOR_PAIR(6));
    wclrtoeol(title_win);
    wrefresh(title_win);
    return 0;
}

int init_ui(){
    int height,width,column_width;
    getmaxyx(stdscr,height,width);

    column_width = width;

    title_win = newwin(2,width,0,0);
    tl_win = newwin(height-4,column_width,2,0);
    state_win = newwin(2,width,height-2,0);

    if(!title_win || !tl_win || !state_win)
        return -1;

    init_title_win();
    init_state_win(STATE_NORMAL);
    init_tl_win();
}

int destroy_ui(){
    if(title_win)
        delwin(title_win);
    if(tl_win)
        delwin(tl_win);
    if(state_win)
        delwin(state_win);
}


int refresh_status_height(WINDOW *win,struct status_node *start,struct status_node *end){
    struct status_node *p;
    for(p = start; p && p != end; p = p->next){
        wmove(win,0,0);
        show_status(win,p);
    }
    return 0;
}


int show_status(WINDOW *win,struct status_node *sn){
    if(!sn)
        return -1;
    status *st = sn->st; 
    if(sn->st->retweeted_status) 
        st = sn->st->retweeted_status;

    init_pair(5, COLOR_GREEN, COLOR_BLACK);
    wattron(win,COLOR_PAIR(5));
    waddstr(win,st->composer->screen_name);
    wattroff(win,COLOR_PAIR(5));
    waddch(win,'\n');

    //waddwstr(win,st->wtext);
    if(st->entity_count == 0)
        waddwstr(win,st->wtext);
    else{
        for(entity *et = st->entities; et!=NULL; et=et->next){
            if(et->type){
                et->type->before_entity(win);
                waddwstr(win,et->text);
                et->type->after_entity(win);
            }
            else
                waddwstr(win,et->text);
        }
    }
    int y,x;
    getyx(win,y,x);
    sn->y_max = y;
    waddch(win,'\n');

    return 0;
}

struct status_node *show_timeline(WINDOW *win, struct status_node *sn,int height, int width){ 
    if(!sn)
        return 0;

    int y,x;
    wclear(win);
    wmove(win,0,0);
    struct status_node *prev = NULL;
    while(sn){
        getyx(win,y,x);
        sn->y_min = y;
        show_status(win,sn);

        if(sn->y_max >= height-3)
            break;
        if(sn == timelines[current_tl_index]->separate)
            draw_border(win,'~');
        else
            draw_border(win,'-');
        prev = sn;
        sn = sn->next;
    }
    wrefresh(win);
    if(sn)
        return sn;
    else
        return prev;
}

int highlight_status(WINDOW *win, struct status_node *sn){
    if(!sn)
        return -1;
    for(int line = sn->y_min; line <= sn->y_max; ++line){
        wmove(win,line,0);
        wchgat(win,-1,A_REVERSE,0,NULL);
    }
    wrefresh(win);

    return 0;
}

int input_new_tweet(WINDOW *win, wchar_t *newtext){
    wclear(win);
    wrefresh(win);
    wmove(win,0,0);
    waddwstr(win,newtext);

    int count = wcslen(newtext);
    newtext += count;
    int y,x;
    char count_state[8] = "";
    sprintf(count_state,"%3d/%d",count,TWEET_MAX_LEN);
    notify_state_change(count_state);

    keypad(win,TRUE);
    echo();
    curs_set(1);
    wchar_t wch = 0;
    while(wget_wch(win, &wch) != ERR && wch != '\n'){
        if(wch == KEY_BACKSPACE){
            if(count == 0)
                continue;
            count --;
            if(count < 140){ 
                getyx(win,y,x);
                if(x == 0){
                    int cols,lines;
                    getmaxyx(win,lines,cols);
                    wmove(win,y-1,cols-2);
                }
                int width = wcwidth(*(newtext-1));
                if(width > 1)
                    for(int i = 0;i<width-1;i++)
                        waddch(win,'\b');
                wclrtoeol(win);
                *(--newtext) = '\0';
            }
        }
        else{
            count ++;
            if(count <= 140)
                *newtext ++ = wch;
        }
        wrefresh(win);
        sprintf(count_state,"%3d/%d",count,TWEET_MAX_LEN);
        notify_state_change(count_state);
    }

    curs_set(0);
    noecho();
    return count;
}
