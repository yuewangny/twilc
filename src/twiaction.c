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
#include <pthread.h>
#include <sys/time.h>
#include <time.h>
#include <errno.h>

#include "twiauth.h"
#include "twierror.h"
#include "twiaction.h"

#define WAIT_SECONDS 20

char home_api_base[] = "http://api.twitter.com/1/statuses/home_timeline.xml";
char mention_api_base[] = "http://api.twitter.com/1/statuses/mentions.xml";
char user_api_base[] = "http://api.twitter.com/1/statuses/user_timeline.xml";


pthread_cond_t get_timeline_condition = PTHREAD_COND_INITIALIZER;
pthread_mutex_t get_timeline_mutex = PTHREAD_MUTEX_INITIALIZER;

// Timeline types : 0 for home, 1 for mentions, 2 for user
char *timeline_api_base[3] = {home_api_base, mention_api_base, user_api_base};

char *fetch_timeline(int timeline_type, char *since_id, char *max_id, char *count){
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
    char *result = oauth_get(timeline_api_base[timeline_type],params,nr_params);
    pthread_cond_signal(&get_timeline_condition);
    return result;
}

void *get_timeline_thread_func(void *arg){
    get_timeline_arg *tmparg = (get_timeline_arg *)arg;
    return fetch_timeline(tmparg->timeline_type,tmparg->since_id,tmparg->max_id,tmparg->count);
}


char *get_timeline(int timeline_type, char *since_id, char *max_id, char *count){
    get_timeline_arg *arg = malloc(sizeof(get_timeline_arg));
    if(arg == NULL){
        SET_ERROR_NUMBER(ERROR_MALLOC);
        return NULL;
    }

    arg->timeline_type = timeline_type;
    arg->since_id = since_id;
    arg->max_id = max_id;
    arg->count = count;

    pthread_t get_timeline_thread;
    int res = pthread_create(&get_timeline_thread,NULL,get_timeline_thread_func,(void *)arg);
    if(res){
        SET_ERROR_NUMBER(ERROR_THREAD);
        return NULL;
    }

    struct timespec abstime;
    struct timeval tp;
    gettimeofday(&tp,NULL);
    abstime.tv_sec = tp.tv_sec;
    abstime.tv_nsec = tp.tv_usec * 1000;
    abstime.tv_sec += WAIT_SECONDS;
    res = pthread_cond_timedwait(&get_timeline_condition,&get_timeline_mutex,&abstime);
    if(res == ETIMEDOUT){
        SET_ERROR_NUMBER(ERROR_NETWORK_CONNECTION);
        pthread_cancel(get_timeline_thread);
        return NULL;
    }

    void *result = NULL;
    pthread_join(get_timeline_thread,&result);
    return (char *)result;
}


