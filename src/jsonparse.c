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

#include <wchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <jansson.h>
#include <string.h>

#include "streaming.h"
#include "event.h"
#include "twitter.h"
#include "ui.h"
#include "entity.h"

int parse_friends_json(json_t *friends_root, void *data_ptr){
    json_t *friend_ids = json_object_get(friends_root,"friends");
    if(!friend_ids || !json_is_array(friend_ids))
        return -1;

    for(int i=0;i < json_array_size(friend_ids);++i){
        json_t *friend_id = json_array_get(friend_ids,i);
        long long id_val = json_integer_value(friend_id);
    }
    return 0;
}

int parse_user_json(json_t *user_root, user **user_ptr){
    json_t *screen_name = json_object_get(user_root, "screen_name");
    if(!json_is_string(screen_name))
        return -1;

    user *usr = newuser();
    usr->screen_name = strdup(json_string_value(screen_name));

    json_t *protected = json_object_get(user_root, "protected");
    if(json_is_true(protected))
        SET_PROTECTED(usr->extra_info);
    *user_ptr = usr;
    return 0;
}

int parse_entity_string(json_t *entity_root, entity *et, status *st){
    json_t *indices = json_object_get(entity_root, "indices");
    if(!json_is_array(indices))
        return -1;
    et->start = json_integer_value(json_array_get(indices,0));
    et->end = json_integer_value(json_array_get(indices,1));
    et->text = malloc(sizeof(wchar_t)*(et->end - et->start + 1));
    memcpy(et->text,st->wtext + et->start,(et->end - et->start)*sizeof(wchar_t));
    et->text[et->end - et->start] = '\0';

    return 0;
}

int parse_entity_json(json_t *entity_root, entity *et, status *st){
    if(!et || !entity_root || !st)
        return -1;

    parse_entity_string(entity_root,et,st);
    et->data = NULL;

    if(et->type == ENTITY_TYPE_MENTION){
    //todo
    }

    return 0;
}

const char *entities_keys[3] = {"user_mentions","hashtags","urls"};

int parse_status_json(json_t *status_root, status **status_ptr){
    json_t *idstr = json_object_get(status_root, "id_str");
    if(!json_is_string(idstr))
        return -1;
    json_t *textstr = json_object_get(status_root, "text");
    if(!json_is_string(textstr))
        return -1;

    status *st = newstatus();
    st->id = strdup(json_string_value(idstr));
    //json_decref(idstr);
    const char *text = json_string_value(textstr);
    st->wtext = malloc(sizeof(wchar_t)*(TWEET_MAX_LEN+1));
    mbstowcs(st->wtext, text, TWEET_MAX_LEN+1);
    //json_decref(textstr);
    st->length = wcslen(st->wtext);
    parse_user_json(json_object_get(status_root, "user"),&(st->composer));

    //printf("\n@%s --  %ls\n", st->composer->screen_name, st->wtext);

    json_t *entities = json_object_get(status_root, "entities");
    if(!entities)
        return -1;

    entity *prev = NULL;
    for(int i = 0; i < ENTITY_TYPE_COUNT; i++){
        json_t *entities_array = json_object_get(entities,entities_keys[i]);
        if(json_is_array(entities_array) && json_array_size(entities_array) > 0){
            for(int j = 0; j < json_array_size(entities_array); ++j){
                json_t *entity_root = json_array_get(entities_array, j);
                if(!entity_root)
                    continue;

                entity *et = newentity();
                et->type = entity_types[i];
                if(parse_entity_json(entity_root,et,st) < 0)
                    destroy_entity(et);
                else{
                    st->entity_count ++;
                    if(!(st->entities))
                        st->entities = et;
                    else
                        prev->next = et;
                    prev = et;
                }
            }
        }
    }

    split_status_entities(st);
    
    json_t *retweeted_status_root = json_object_get(status_root,"retweeted_status");
    if(retweeted_status_root){ 
        int result = parse_status_json(retweeted_status_root,&(st->retweeted_status));
        if(result == -1)
            st->retweeted_status = NULL;
    }
    /*
    for(entity *et = st->entities;et;et=et->next)
        printf("%ls ",et->text);
    printf("\n");
    */

    *status_ptr = st;

    return 0;
}

int parse_event_json(json_t *event_root, event **event_ptr){
}

