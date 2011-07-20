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


#ifndef TWITTER_H
#define TWITTER_H

#include <stdint.h>
#include <pthread.h>
#include <glib.h>
#include <wchar.h>

#include "entity.h"
#include "config.h"

// user
typedef struct {
    char *id;
    char *screen_name;
    char *name;

    char *location;
    char *bio;
    char *url;

    uint8_t extra_info; //protected,following,followed
} user;

// entity types
#define ENTITY_MENTION_TYPE 0 
#define ENTITY_URL_TYPE 1
#define ENTITY_HASHTAG_TYPE 2


#define TWEET_MAX_LEN 140

// status
typedef struct status_str{
    // content of the status
    char *id;
    user *composer;
    wchar_t *wtext;
    struct status_str *retweeted_status;
    uint8_t length; //number of characters

    // in reply to
    char *in_reply_to_status_id;
    void *conversation; // status_node *conversation

    // extra information
    uint8_t extra_info;

    entity *entities;
    int entity_count;
} status;

// displayed status
struct status_node{
    status *st;

    struct status_node *prev;
    struct status_node *next;

    // position to show
    int y_min;
    int y_max;
};

// timeline
// 28 bytes
typedef struct{
    struct status_node *head;
    int count;
    struct status_node *current;
    struct status_node *current_top;
    struct status_node *current_bottom;
    struct status_node *last_viewed;
    struct status_node *separate;
} statuses;

#define IS_SEPARATED(a)  a & 0x80
#define SET_SEPARATED(a) a |= 0x80
#define UNSET_SEPARATED(a) a &= 0x7F

#define IS_MENTIONED(a)  a & 0x40
#define SET_MENTIONED(a) a |= 0x40

#define IS_RETWEETED(a) a & 0x20
#define SET_RETWEETED(a) a |= 0x20

#define IS_FROMSELF(a) a & 0x10
#define SET_FROMSELF(a) a |= 0x10

#define IS_FAVORITED(a) a & 0x08
#define SET_FAVORITED(a) a |= 0x08
#define UNSET_FAVORITED(a) a &= 0xF7

#define IS_RETWEETEDBYSELF a & 0x04
#define SET_RETWEETEDBYSELF a |= 0x04
#define UNSET_RETWEETEDBYSELF a &= 0xFB

#define TIMELINE_COUNT 1 //0 for home, 1 for mention

statuses *timelines[TIMELINE_COUNT];

// status hashmap
GHashTable *status_map;
pthread_mutex_t status_map_mutex;

// user hashmap
GHashTable *user_map;
pthread_mutex_t user_map_mutex;

user *me;
// the index of current displaying timeline
int current_tl_index;

int authorize(clit_config *config);
int update_timeline(int tl_index, struct status_node *from_status, struct status_node *to_status);
int load_timeline(char *tmpfile, statuses *tl,struct status_node *from,struct status_node *to);
void build_status_entities(status *s);


user *newuser();
entity *newentity();
int destroy_user();
status *newstatus();
void split_status_entities(status *st);
struct status_node *newstatusnode(status *st);
int init_timelines();
int destroy_timeline(statuses *tl);
int destroy_status(status *s);

#endif
