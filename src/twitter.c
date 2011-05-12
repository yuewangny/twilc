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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libxml/parser.h>

#include "twiauth.h"
#include "config.h"
#include "twitter.h"
#include "filter.h"
#include "twiparse.h"
#include "twiaction.h"
#include "ui.h"
#include "twierror.h"

#define GAP_STATUS_ID 0
#define GAP_STATUS_TEXT "gap"

#define DEFAUTL_REFRESH_COUNT "50"
#define DEFAULT_LOAD_COUNT "200"

status *newgapstatus(){
    status *s = newstatus();
    s->id = GAP_STATUS_ID;
    s->text = GAP_STATUS_TEXT;
    s->filter_count = 0;

    return s;
}

user *newuser(){
    user *usr = malloc(sizeof(user));
    usr->id = 0;
    usr->screen_name = 0;
    return usr;
}

status *newstatus(){
    status *s = malloc(sizeof(status));
    s->id = 0;
    s->composer = 0;
    s->retweeter = 0;
    s->extra_info = 0;
    s->text = 0;
    s->prev = 0;
    s->next = 0;
    return s;
}

statuses *newtimeline(){
    statuses *tl = malloc(sizeof(statuses));
    tl->head = 0;
    tl->count = 0;
    return tl;
}

int update_timeline(int tl_index, status *from_status, status *to_status){
    char *since_id = 0;
    char *max_id = 0;

    if(from_status)
        max_id = from_status->id;
    if(to_status){
        if(to_status->next)
            since_id = to_status->next->id;
        else
            since_id = to_status->id;
    }
    char *tmpfile;
    tmpfile = get_timeline(tl_index,since_id,max_id,DEFAUTL_REFRESH_COUNT);
    if(!tmpfile)
        return -1;
    notify_state_change(states[STATE_LOADING_UPDATES]);
    int nr_newtweets = load_timeline(tmpfile,timelines[current_tl_index],from_status,to_status);
    remove(tmpfile);

    return nr_newtweets;
}

int init_timelines(){
    for(int i = 0; i < TIMELINE_COUNT; ++i){
        timelines[i] = newtimeline();
    }
    statuses *home = timelines[0];
    char *tmpfile = get_timeline(TL_TYPE_HOME, NULL,NULL,DEFAULT_LOAD_COUNT);
    if(!tmpfile)
        return -1;
    load_timeline(tmpfile,home,NULL,NULL);
    remove(tmpfile);
    current_status[0] = timelines[0]->head;
    current_top_status[0] = timelines[0]->head;
    last_viewed_status[0] = NULL;

    current_tl_index = 0;

    return 0;
}

int destroy_timeline(statuses *tl){
    if(!tl)
        return 0;
    status *p = tl->head;
    while(p){
        status *tmp = p;
        p = p->next;
        destroy_status(tmp);
    }
    free(tl);
    return 0;
}

int destroy_user(user *usr){
    if(usr){
        if(usr->id)free(usr->id);
        if(usr->screen_name)free(usr->screen_name);
        free(usr);
    }
    return 0;
}

int destroy_status(status *s){
    if(!s)
        return 0;
    if(s->id){
        free(s->id);
        for(int i=0;i<s->filter_count;i++)
            free(s->filtered_text[i]);
        destroy_user(s->composer);
        destroy_user(s->retweeter);
    }
    free(s);
    return 0;
}

int authorize(clit_config *config){
    char *access_token;
    char *access_token_secret;
    char *user_id;
    char *screen_name;

    if(oauth_authorize(&access_token,&access_token_secret,&user_id,&screen_name)){
        printf("Authorization failed\n");
        return -1;
    }
    else{
        init_config(access_token,access_token_secret,user_id,screen_name,config);
        save_config(config);
        return 0;
    }
}

void filter_status_text(status *s){
    if(!s)
        return;

    display_filter *current_filter;
    char **filtered_text = s->filtered_text;
    display_filter **filter_list = s->filter_list;

    int i = 0;
    char *begin = NULL;
    for(int k=0;k<FILTER_NUM;++k){
        char *temp = strstr(s->text,filters[k]->pattern);
        if(temp && (!begin || (begin && temp < begin))){
            begin = temp;
            current_filter = filters[k];
        }
    }

    char *end = s->text;
    char *prev = s->text;
    while(begin){
        prev = end;
        end = current_filter->get_pattern_end(begin);
        if(end == 0)
            break;

        int len = begin - prev;
        if(len > 0){
            filtered_text[i] = malloc((len + 1)*sizeof(char));
            strncpy(filtered_text[i],prev,len);
            filtered_text[i][len] = '\0';
            filter_list[i] = 0;
            i++;
        }

        len = end - begin;
        filtered_text[i] = malloc((len + 1)*sizeof(char));
        strncpy(filtered_text[i],begin,len);
        filtered_text[i][len] = '\0';
        filter_list[i] = current_filter;
        i++;

        begin = NULL;
        for(int k=0;k<FILTER_NUM;++k){
            char *temp = strstr(end,filters[k]->pattern);
            if(temp && (!begin || (begin && temp < begin))){
                begin = temp;
                current_filter = filters[k];
            }
        }
    }
    if(end)
        prev = end;
    if((*prev) != '\0'){
        filtered_text[i] = malloc((strlen(prev)+1)*sizeof(char));
        strcpy(filtered_text[i],prev);
        filter_list[i] = 0;
        i ++;
    }
    s->filter_count = i;

    for(int i=0;i<s->filter_count;++i)
        if(filtered_text[i][0] == '@' && strcmp(filtered_text[i]+1,s->composer->screen_name) == 0){
            SET_MENTIONED(s->extra_info);
            break;
        }
}

int change_separate_status(status *newsep){
    status *s = separate_status[current_tl_index];
    if(s)
        UNSET_SEPARATED(s->extra_info);
    separate_status[current_tl_index] = newsep;
    SET_SEPARATED(newsep->extra_info);
}

/*
 * Merge new tweets with current timeline.
 */
int load_timeline(char *tmpfile, statuses *tl, status *from_status, status *to_status){
    LIBXML_TEST_VERSION
    statuses *toptweets = newtimeline();

    if(parse_timeline(tmpfile,toptweets) < 0)
        return -1;

    for(status *s = toptweets->head; s; s = s->next){
        filter_status_text(s);
    }

    if(!(toptweets->head) || toptweets->count == 0)
        return 0;
    if(tl->count == 0){ // No old tweets
        tl->head = toptweets->head;
        tl->count = toptweets->count;
        return tl->count;
    }
    else{
        int nr_newtweets = 0;
        status *top = toptweets->head;
        status *oldtop = to_status;
        if(oldtop){
            if(strcmp(top->id,oldtop->id) <= 0)
                return 0;

            status *prev = top->prev;
            while(top){
                int result = strcmp(top->id,oldtop->id);
                if(result > 0){ // new tweet
                    prev = top;
                    top = top->next;
                    nr_newtweets ++;
                }
                else{
                    if(prev){
                        prev->next = oldtop;
                        oldtop->prev = prev;
                        change_separate_status(prev);
                    }
                    break;
                }
            }
            if(nr_newtweets == 0)
                return 0;

            // Gap between new and old tweets
            if(!top){
                status *gap = newgapstatus();
                prev->next = gap;
                gap->prev = prev;
                gap->next = oldtop;
                oldtop->prev = gap;
            }
        }

        // free deleted tweets
        //status *p = tl->head;
        //while(p && p != oldtop){
        //prev = p;
        //p = p->next;
        //free(prev);
        //}

        if(from_status){
            from_status->next = toptweets->head;
            toptweets->head->prev = from_status;
        }
        else{
            tl->head = toptweets->head;
            tl->head->prev = NULL;
        }

        return nr_newtweets;
    }
}


