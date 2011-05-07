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

#include "twiauth.h"

char home_api_base[] = "http://api.twitter.com/1/statuses/home_timeline.xml";
char mention_api_base[] = "http://api.twitter.com/1/statuses/mentions.xml";
char user_api_base[] = "http://api.twitter.com/1/statuses/user_timeline.xml";

// Timeline types : 0 for home, 1 for mentions, 2 for user
char *timeline_api_base[3] = {home_api_base, mention_api_base, user_api_base};



char *get_timeline(int timeline_type, char *since_id, char *max_id, char *count){
    int nr_params = 1;
    kvpair *params = NULL;

    if(since_id)
        nr_params ++;
    if(max_id)
        nr_params ++;
    params = malloc(nr_params*sizeof(kvpair));

    int i = 0;
    if(since_id){
        params[i].key = "since_id";
        params[i].value = since_id;
        i++;
    }
    if(max_id){
        params[i].key = "max_id";
        params[i].value = max_id;
        i++;
    }
    params[i].key = "count";
    params[i].value = count;
    i++;
    return oauth_get(timeline_api_base[timeline_type],params,nr_params);
}

