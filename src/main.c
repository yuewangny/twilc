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
#include <pthread.h>

#include "twitter.h"
#include "config.h"
#include "twiauth.h"
#include "twiaction.h"
#include "entity.h"
#include "twiparse.h"
#include "ui.h"
#include "twierror.h"

int return_to_current_timeline(WINDOW *win){
    int y,x;
    getmaxyx(win,y,x);

    show_timeline(win,timelines[current_tl_index]->current_top,y,x);
    highlight_status(win,timelines[current_tl_index]->current);

    return 0;
}

int move_next_page(WINDOW *win, struct status_node *page_start, int direction){
    if(!page_start){
        SET_ERROR_NUMBER(ERROR_PAGE_START);
        return -1;
    }

    struct status_node *top = 0;
    struct status_node *bottom = 0;
    int y,x;

    getmaxyx(win,y,x);
    if(direction > 0){
        top = page_start;
    }
    else{
        struct status_node *p = page_start;
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

    timelines[current_tl_index]->current_top = top;
    timelines[current_tl_index]->current_bottom = bottom;
    return 0;
}

void move_next(WINDOW *win, struct status_node *current, int direction){
    if(!current)
        return;

    struct status_node *next = 0;
    struct status_node *boundary = 0;
    if(direction > 0){
        next = current->next;
        boundary = timelines[current_tl_index]->current_bottom;
    }
    else{
        next = current->prev;
        boundary = timelines[current_tl_index]->current_top;
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
    timelines[current_tl_index]->current = next;
}

void move_top(WINDOW *win){
    struct status_node *top = timelines[current_tl_index]->head;
    if(!top)
        return;

    int y,x;
    getmaxyx(win,y,x);
    struct status_node *bottom = show_timeline(win,top,y,x);
    timelines[current_tl_index]->current = top;
    timelines[current_tl_index]->current_top = top;
    timelines[current_tl_index]->current_bottom = bottom;
    highlight_status(win,top);
}

int compose_new_tweet(WINDOW *win,status *in_reply_to, int if_reply_to_all){
    wchar_t *newtext = malloc((1+TWEET_MAX_LEN)*sizeof(wchar_t));
    memset(newtext,'\0',(1+TWEET_MAX_LEN)*sizeof(wchar_t));
    if(in_reply_to){
        if(in_reply_to->retweeted_status)
            in_reply_to = in_reply_to->retweeted_status;
        wchar_t *ptr = newtext;
        *ptr ++ = '@';
        mbstowcs(ptr,in_reply_to->composer->screen_name,strlen(in_reply_to->composer->screen_name));
        ptr += wcslen(ptr);
        *ptr ++ = ' ';
        if(if_reply_to_all)
            for(entity *et = in_reply_to->entities; et; et = et->next){
                if(et->type == ENTITY_TYPE_MENTION){
                    wcscpy(ptr,et->text);
                    ptr += wcslen(ptr);
                    *ptr = ' ';
                    ++ ptr;
                }
            }
    }
    int count = input_new_tweet(win,newtext);
    if(count > 0){
        char text[TWEET_MAX_LEN * 4];
        char *in_reply_to_status_id = NULL;
        if(in_reply_to)
            in_reply_to_status_id = in_reply_to->id;
        wcstombs(text,newtext,TWEET_MAX_LEN * 4);
        notify_state_change("Sending......");
        update_status(text,in_reply_to_status_id);
        notify_state_change("Successfully updated.");
    }
    else
        notify_state_change("");
    free(newtext);
    return 0;
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
 * r -- reply
 * R -- reply to all
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
                compose_new_tweet(win,NULL,0);
                return_to_current_timeline(win);
                break;
            case 'J':
                // move down one page
                notify_state_change(states[STATE_NORMAL]);
                if(move_next_page(win,timelines[current_tl_index]->current_bottom,1) != -1){
                    timelines[current_tl_index]->current = timelines[current_tl_index]->current_top;
                    highlight_status(win, timelines[current_tl_index]->current);
                }
                else
                    notify_error_state();
                break;
            case 'K':
                // move up one page
                notify_state_change(states[STATE_NORMAL]);
                if(move_next_page(win,timelines[current_tl_index]->current_top,-1)!= -1){
                    timelines[current_tl_index]->current = timelines[current_tl_index]->current_top;
                    highlight_status(win, timelines[current_tl_index]->current);
                }
                else
                    notify_error_state();
                break;
            case 'j':
                // move down one tweet
                notify_state_change(states[STATE_NORMAL]);
                move_next(win, timelines[current_tl_index]->current,1);
                break;
            case 'k':
                // move up one tweet
                notify_state_change(states[STATE_NORMAL]);
                move_next(win, timelines[current_tl_index]->current,-1);
                break;
            case '.':
                // refresh the current timeline
                notify_state_change(states[STATE_RETRIEVING_UPDATES]);
                struct status_node *to_status = timelines[current_tl_index]->head;
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
                    notify_error_state();

                timelines[current_tl_index]->last_viewed = timelines[current_tl_index]->current;
                move_top(win);
                break;
            case 'v':
                notify_state_change(states[STATE_NORMAL]);
                if(timelines[current_tl_index]->last_viewed){
                    int y,x;
                    struct status_node *last_viewed = timelines[current_tl_index]->last_viewed;
                    getmaxyx(win,y,x);
                    timelines[current_tl_index]->current = last_viewed;
                    timelines[current_tl_index]->current_top = last_viewed;
                    timelines[current_tl_index]->current_bottom = show_timeline(win,last_viewed,y,x);
                    highlight_status(win,last_viewed);
                }
                else
                    notify_state_change(states[STATE_NO_LAST_VIEWED]);
                break;
            case 'r':
                compose_new_tweet(win,timelines[current_tl_index]->current->st,0);
                return_to_current_timeline(win);
                break;
        }
    }
}

int init_mutex(){
    pthread_mutex_init(&error_mutex,NULL);
    return 0;
}

int destroy_mutex(){
    pthread_mutex_destroy(&error_mutex);
    return 0;
}

int main(){
    setlocale(LC_ALL,"");

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

    me = newuser();
    me->screen_name = config->screen_name;
    me->id = config->user_id;

    init_oauth(config->key,config->secret);
    free(config);

    printf("Loading the timelines....\n");
    if(init_timelines() == -1){
        pthread_mutex_lock(&error_mutex);
        printf("Cannot load the timeline. Please check your network connection.\n");
        pthread_mutex_unlock(&error_mutex);
        goto exit_twilc;
    }
    init_mutex();

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
    destroy_mutex();
    for(int i = 0; i < TIMELINE_COUNT; ++i)
        destroy_timeline(timelines[i]);

    return 0;
}
