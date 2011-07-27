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
#include <glib.h>
#include <wchar.h>

#include "twiauth.h"
#include "config.h"
#include "twitter.h"
#include "entity.h"
#include "twiparse.h"
#include "twiaction.h"
#include "ui.h"
#include "twierror.h"

#define GAP_STATUS_ID 0
#define GAP_STATUS_TEXT "gap"

#define DEFAUTL_REFRESH_COUNT "50"
#define DEFAULT_LOAD_COUNT "200"
#define STATUS_MAX_NUM 2000



status *newgapstatus(){
    status *s = newstatus();
    s->id = GAP_STATUS_ID;
    s->wtext = L"GAP";
    return s;
}

user *newuser(){
    user *usr = malloc(sizeof(user));
    usr->id = 0;
    usr->screen_name = 0;
    usr->name = 0;
    usr->location = 0;
    usr->url = 0;
    usr->bio = 0;
    return usr;
}

status *newstatus(){
    status *s = malloc(sizeof(status));
    s->id = 0;
    s->composer = 0;
    s->retweeted_status = 0;
    s->extra_info = 0;
    s->wtext = 0;
    s->length = 0;
    s->in_reply_to_status_id = 0;
    s->conversation = 0;

    s->entities = 0;
    s->entity_count = 0;
    return s;
}

struct status_node *newstatusnode(status *st){
    struct status_node *sn = malloc(sizeof(struct status_node));
    if(!sn){
        SET_ERROR_NUMBER(ERROR_MALLOC);
        return NULL;
    }
    sn->prev = 0;
    sn->next = 0;

    sn->y_min = -1;
    sn->y_max = -1;
    sn->st = st;

    return sn;
}

int destroy_statusnode(struct status_node *sn){
    free(sn);
}

void split_status_entities(status *st){
    if(st->entity_count == 0){
        return;
    }
    entity *entities = st->entities;
    entity *flag[st->length+1];
    memset(flag,0,(st->length+1)*sizeof(entity *));

    for(entity *e = entities; e!=NULL; e=e->next){
        flag[e->start] = e;
        flag[e->end] = e;
    }
    int i = 0, j;
    entity *et = NULL;
    for(j = i+1; j <= st->length; j++){
        if(flag[j] || j == st->length){
            entity *tmp =NULL;
            if(flag[j] != flag[i]){
                tmp = newentity();
                tmp->text = malloc((j-i+1)*sizeof(wchar_t));
                memcpy(tmp->text,st->wtext+i,(j-i)*sizeof(wchar_t));
                tmp->text[j-i] = '\0';
                tmp->start = i;
                tmp->end = j;
                tmp->type = NULL;
            }
            else
                tmp = flag[j];
            if(et){
                et->next = tmp;
                et = et->next;
            }
            else{
                et = tmp;
                st->entities = et;
            }
            i = j;
        }
    }
    et->next = NULL;
}

statuses *newtimeline(){
    statuses *tl = malloc(sizeof(statuses));
    tl->head = 0;
    tl->count = 0;
    tl->current = 0;
    tl->current_top = 0;
    tl->current_bottom = 0;
    tl->separate = 0;
    tl->last_viewed = 0;
    tl->updates = malloc(sizeof(statuses_updates));
    tl->updates->head = NULL;
    tl->updates->tail = NULL;
    tl->updates->count = 0;

    pthread_mutex_init(&(tl->timeline_mutex),NULL);
    pthread_mutex_init(&(tl->updates_mutex),NULL);
    return tl;
}

int add_status(status *st, statuses *timeline){
    if(!st)
        return -1;

    int updates_count = 0;
    pthread_mutex_lock(&(timeline->updates_mutex));
    struct status_node *sn = newstatusnode(st);
    sn->next = timeline->updates->head;
    if(timeline->updates->count == 0)
        timeline->updates->tail = sn;
    else
        timeline->updates->head->prev = sn;
    timeline->updates->head = sn;
    timeline->updates->count ++;
    updates_count = timeline->updates->count;
    pthread_mutex_unlock(&(timeline->updates_mutex));

    return updates_count;
}

/*
int load_conversation(struct status_node *sn){
    status *in_reply_to_status;
    while(sn->st->in_reply_to_status_id){
        status *s = sn->st;
        if(!s->conversation){
            pthread_mutex_lock(&status_map_mutex);
            in_reply_to_status = g_hash_table_lookup(s->in_reply_to_status_id);
            pthread_mutex_unlock(&status_map_mutex);
            if(!in_reply_to_status){
                in_reply_to_status = parse_single_status(get_single_status(s->in_reply_to_status_id));
            }
            if(!in_reply_to_status){
                break;
            }
            struct status_node *conversation = newstatusnode(in_reply_to_status);
        }
        sn = (struct status_node *)(s->conversation);
    }
    return 0;
}
*/

int merge_timeline_updates(int tl_index){
    statuses *tl = timelines[tl_index];
    int updates_count;

    pthread_mutex_lock(&(tl->updates_mutex));
    pthread_mutex_lock(&(tl->timeline_mutex));
    updates_count = tl->updates->count;
    if(updates_count != 0){
        tl->updates->tail->next = tl->head;
        tl->head->prev = tl->updates->tail;
        tl->head = tl->updates->head;
        tl->count += tl->updates->count;
        tl->separate = tl->updates->tail;

        tl->updates->head = tl->updates->tail = NULL;
        tl->updates->count = 0;
    }
    pthread_mutex_unlock(&(tl->updates_mutex));
    pthread_mutex_unlock(&(tl->timeline_mutex));

    return updates_count;
}

int update_timeline(int tl_index, struct status_node *from_status, struct status_node *to_status){
    char *since_id = 0;
    char *max_id = 0;

    if(from_status)
        max_id = from_status->st->id;
    if(to_status){
        if(to_status->next)
            since_id = to_status->next->st->id;
        else
            since_id = to_status->st->id;
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
    init_entity_types();
    for(int i = 0; i < TIMELINE_COUNT; ++i){
        timelines[i] = newtimeline();
    }
    statuses *home = timelines[0];
    char *tmpfile = get_timeline(TL_TYPE_HOME, NULL,NULL,DEFAULT_LOAD_COUNT);
    if(!tmpfile)
        return -1;
    load_timeline(tmpfile,home,NULL,NULL);
    remove(tmpfile);
    home->current = home->head;
    home->current_top = home->head;
    home->last_viewed = NULL;

    /*
     * status_map = g_hash_table_new(g_str_hash,strcmp);
     */

    current_tl_index = 0;

    return 0;
}

int destroy_timeline(statuses *tl){
    if(!tl)
        return 0;
    struct status_node *p = tl->head;
    while(p){
        struct status_node *tmp = p;
        p = p->next;
        free(tmp);
    }
    if(tl->updates){
        p = tl->updates->head;
        while(p){
            struct status_node *tmp = p;
            p = p->next;
            free(tmp);
        }
        free(tl->updates);
    }
    free(tl);
    return 0;
}

int destroy_user(user *usr){
    if(usr){
        if(usr->id)free(usr->id);
        if(usr->screen_name)free(usr->screen_name);
        if(usr->url)free(usr->url);
        if(usr->bio)free(usr->bio);
        if(usr->location)free(usr->location);
        free(usr);
    }
    return 0;
}


/*
int destroy_conversation(struct status_node* conv){
    while(conv){
        struct status_node *tmp = conv;
        conv = conv->next;
        free(tmp);
    }
    return 0;
}
*/

int destroy_status(status *s){
    if(!s)
        return 0;
    printf("Destroying status.......");
    if(s->id){
        free(s->id);
        free(s->wtext);
        if(s->in_reply_to_status_id)
            free(s->in_reply_to_status_id);
        /*
        if(s->conversation)
            destroy_conversation((struct status_node*)conversation);
        */
        entity *et=s->entities;
        while(et){
            entity *prev = et;
            et = et->next;
            destroy_entity(prev);
        }
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

int change_separate_status(struct status_node *newsep){
    struct status_node *s = timelines[current_tl_index]->separate;
    if(s)
        UNSET_SEPARATED(s->st->extra_info);
    timelines[current_tl_index]->separate = newsep;
    SET_SEPARATED(newsep->st->extra_info);
}

/*
 * Merge new tweets with current timeline.
 */
int load_timeline(char *tmpfile, statuses *tl, struct status_node *from_status, struct status_node *to_status){
    LIBXML_TEST_VERSION
    statuses *toptweets = newtimeline();

    if(parse_timeline(tmpfile,toptweets) < 0)
        return -1;

    /*
    for(struct status_node *s = toptweets->head; s; s = s->next)
        get_status_entities(s->st);
        */

    if(!(toptweets->head) || toptweets->count == 0)
        return 0;
    if(tl->count == 0){ // No old tweets
        tl->head = toptweets->head;
        tl->count = toptweets->count;
        refresh_status_height(tl_win,tl->head,NULL);
        return tl->count;
    }
    else{
        int nr_newtweets = 0;
        struct status_node *top = toptweets->head;
        struct status_node *oldtop = to_status;
        if(oldtop){
            if(strcmp(top->st->id,oldtop->st->id) <= 0)
                return 0;

            struct status_node *prev = top->prev;
            while(top){
                int result = strcmp(top->st->id,oldtop->st->id);
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
                struct status_node *gap = newstatusnode(newgapstatus());
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

        refresh_status_height(tl_win,toptweets->head,oldtop);
        return nr_newtweets;
    }
}


