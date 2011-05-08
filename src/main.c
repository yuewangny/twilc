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

int move_next_page(WINDOW *win, status *page_start, int direction){
    if(!page_start)
        return -1;

    status *top = 0;
    status *bottom = 0;
    int y,x;

    getmaxyx(win,y,x);
    if(direction > 0){
        top = page_start;
    }
    else{
        status *p = page_start;
        int topy = y;
        while(p){
            topy -= p->y_max - p->y_min + 2;
            if(topy <= 0)
                break;
            p = p->prev;
        }
        if(!p)
            p = timelines[current_tl_index]->head;
        else
            p = p->next;
        top = p;
    }
    wclear(win);
    bottom = show_timeline(win,top,y,x);

    current_top_status[current_tl_index] = top;
    current_bottom_status[current_tl_index] = bottom;
    return 0;
}

void move_next(WINDOW *win, status *current, int direction){
    if(!current)
        return;

    status *next = 0;
    status *boundary = 0;
    if(direction > 0){
        next = current->next;
        boundary = current_bottom_status[current_tl_index];
    }
    else{
        next = current->prev;
        boundary = current_top_status[current_tl_index];
    }
    if(!next){  // reached the bottom
        if(direction > 0)
            notify_state_change(states[STATE_REACHED_BOTTOM]);
        else
            notify_state_change(states[STATE_REACHED_TOP]);
        return;
    }

    if(current != boundary){
        wmove(win,current->y_min,0);
        show_status(win,current);
    }
    else
        move_next_page(win,current,direction);

    highlight_status(win,next);
    current_status[current_tl_index] = next;
}

void move_top(WINDOW *win){
    status *top = timelines[current_tl_index]->head;
    if(!top)
        return;

    int y,x;
    getmaxyx(win,y,x);
    status *bottom = show_timeline(win,top,y,x);
    current_status[current_tl_index] = top;
    current_top_status[current_tl_index] = top;
    current_bottom_status[current_tl_index] = bottom;
    highlight_status(win,top);
}

/**
 * Commands:
 * a -- 
 * b -- 
 * c -- conversation
 * d -- delete a tweet
 * e --
 * f -- fav/unfav a tweet
 * g --
 * h -- 
 * i --
 * j -- move down one tweet
 * J -- move down one page
 * k -- move up one tweet
 * K -- move up one page
 * l -- list
 * m -- direct message
 * n -- compose a new tweet
 * o --
 * p --
 * q -- quit
 * r -- reply to a tweet
 * s -- search
 * t -- retweet
 * u --
 * v -- move to the last viewed tweet
 * w --
 * x --
 * y -- 
 * z --
 * ? -- show shortcuts help
 * . -- refresh
 */
void wait_command(WINDOW *win){
    char ch = '\0';
    while((ch = getch()) != 'q'){
        switch(ch){
            case 'n':
                // Compose new tweet
                notify_state_change(states[STATE_NORMAL]);
                break;
            case 'J':
                // move down one page
                notify_state_change(states[STATE_NORMAL]);
                if(move_next_page(win,current_bottom_status[current_tl_index],1) != -1){
                    current_status[current_tl_index] = current_top_status[current_tl_index];
                    highlight_status(win, current_status[current_tl_index]);
                }
                break;
            case 'K':
                // move up one page
                notify_state_change(states[STATE_NORMAL]);
                if(move_next_page(win,current_top_status[current_tl_index],-1)!= -1){
                    current_status[current_tl_index] = current_top_status[current_tl_index];
                    highlight_status(win, current_status[current_tl_index]);
                }
                break;
            case 'j':
                // move down one tweet
                notify_state_change(states[STATE_NORMAL]);
                move_next(win, current_status[current_tl_index],1);
                break;
            case 'k':
                // move up one tweet
                notify_state_change(states[STATE_NORMAL]);
                move_next(win, current_status[current_tl_index],-1);
                break;
            case '.':
                // refresh the current timeline
                notify_state_change(states[STATE_RETRIEVING_UPDATES]);
                status *to_status = timelines[current_tl_index]->head;
                int res = update_timeline(current_tl_index,NULL,to_status);
                if(res >= 0){
                    char *state_str = malloc(20*sizeof(char));
                    memset(state_str,'\0',20);
                    if(res == 0)
                        sprintf(state_str,"%s","No updates.");
                    else if(res == 1)
                        sprintf(state_str,"%d new tweet.",res);
                    else
                        sprintf(state_str,"%d new tweets.",res);
                    notify_state_change(state_str);
                    free(state_str);
                }
                else
                    notify_state_change(states[STATE_RETRIEVE_FAILED]);

                last_viewed_status[current_tl_index] = current_status[current_tl_index];
                move_top(win);
                break;
            case 'v':
                notify_state_change(STATE_NORMAL);
                if(last_viewed_status[current_tl_index]){
                    int y,x;
                    status *last_viewed = last_viewed_status[current_tl_index];
                    getmaxyx(win,y,x);
                    show_timeline(win,last_viewed,y,x);
                    highlight_status(win,last_viewed);
                    current_status[current_tl_index] = last_viewed;
                }
                break;
        }
    }
}

int main(){
    clit_config *config = malloc(sizeof(clit_config));
    if(parse_config(config) == -1){
        // first run
        int res = authorize(config);
        if(res == -1){
            printf("Please retry.\n");
        }
        else
            printf("Authorization finished. Run the program again.\n");
        free(config);
        exit(0);
    }

    init_oauth(config->key,config->secret);
    create_filters();
    init_timelines();

    setlocale(LC_ALL,"");
    initscr();
    curs_set(0);


    //if(has_colors() == FALSE)
        //goto exit_twilc;
    raw();
    keypad(stdscr,TRUE);
    noecho();
    start_color();

    move(0,0);
    refresh();

    if(init_ui() == -1)
        goto exit_twilc;

    wait_command(tl_win);


exit_twilc:
    destroy_ui();
    curs_set(1);
    endwin(); 
    destroy_filters();
    for(int i = 0; i < TIMELINE_COUNT; ++i)
        destroy_timeline(timelines[i]);

    return 0;
}
