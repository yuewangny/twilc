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
#include <unistd.h>
#include <string.h>
#include <oauth.h>
#include <curl/curl.h>
#include <pthread.h>

#include "twiauth.h"
#include "streaming.h"
#include "twierror.h"
#include "event.h"

extern const char *CONSUMER_KEY;
extern const char *CONSUMER_SECRET;

CURL *connection; 

raw_event_queue *new_raw_event_queue(){
    raw_event_queue *raw_event_stream = malloc(sizeof(raw_event_queue));
    raw_event_stream->head = NULL;
    raw_event_stream->tail = NULL;
    return raw_event_stream;
}

int destroy_raw_event_queue(raw_event_queue *queue){
    if(!queue)
        return 0;
    for(struct raw_event *re = queue->head; re; re = re->next)
        destroy_raw_event(re);

    free(queue);
}

struct raw_event *new_raw_event(char *data){
    struct raw_event *re = malloc(sizeof(struct raw_event));
    re->data = data;
    re->next = NULL;
    return re;
}

int destroy_raw_event(struct raw_event *re){
    if(!re)
        return 0;
    if(re->data)
        free(re->data);
    free(re);
    return 0;
}

int add_raw_event(raw_event_queue *queue, char *data){
    if(data == NULL || strlen(data) == 0)
        return 0;
    if(!queue)
        return -1;

    struct raw_event *re = new_raw_event(data);
    pthread_mutex_lock(&event_buffer_mutex);
    if(queue->head)
        queue->tail->next = re;
    else
        queue->head = re;
    queue->tail = re;
    pthread_mutex_unlock(&event_buffer_mutex);

    return 0;
}

char *extract_raw_event(raw_event_queue *queue){
    if(!queue)
        return NULL;
    
    char *data = NULL;
    struct raw_event *temp = NULL;
    pthread_mutex_lock(&event_buffer_mutex);
    if(queue && queue->head){
        temp = queue->head;
        data = temp->data;
        queue->head = temp->next;
        if(queue->head == NULL)
            queue->tail = NULL;
    }
    pthread_mutex_unlock(&event_buffer_mutex);

    if(temp)
        free(temp);
    return data;
}

size_t callback_func(void *ptr, size_t size, size_t count, void *stream){
    char *data = malloc(strlen((char *)ptr)+1);
    strcpy(data,(char *)ptr);

    add_raw_event(raw_event_stream, data);

    return count;
}

int open_userstream_conn(){
    char *url = oauth_sign_url2(USER_STREAM_API_BASE,NULL,OA_HMAC,NULL,CONSUMER_KEY, CONSUMER_SECRET,ACCESS_TOKEN,ACCESS_TOKEN_SECRET);

    curl_global_init(CURL_GLOBAL_SSL);
    connection = curl_easy_init();
    CURLcode res;
    if(connection){
        curl_easy_setopt(connection,CURLOPT_URL,url);
        curl_easy_setopt(connection,CURLOPT_WRITEFUNCTION,callback_func);
        //curl_easy_setopt(connection,CURLOPT_VERBOSE,1);

        CURLcode res = curl_easy_perform(connection);

        return 0;
    }
    else{
        SET_ERROR_NUMBER(ERROR_NETWORK_CONNECTION);
        return -1;
    }
}

int close_userstream_conn(){
    curl_easy_cleanup(connection);
}

void *collecting_thread_func(void *arg){
    open_userstream_conn();
    return NULL;
}

void *consuming_thread_func(void *arg){
    while(1){
        char *data = extract_raw_event(raw_event_stream);
        if(data == NULL){
            usleep(500);
        }
        else{
            consume_stream(data);
            free(data);
        }
    }
    return NULL;
}

int start_userstream(){
    pthread_create(&collecting_thread, NULL, collecting_thread_func, NULL);
    pthread_create(&consuming_thread, NULL, consuming_thread_func, NULL);
}

int stop_userstream(){
    close_userstream_conn();
}
