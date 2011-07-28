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

#ifndef STREAMING_H
#define STREAMING_H

#define USER_STREAM_API_BASE "https://userstream.twitter.com/2/user.json"

#include <pthread.h>
#include "twitter.h"

int open_userstream_conn();
int close_userstream_conn();

typedef struct raw_event {
    char *data;
    struct raw_event *next;
}; 

typedef struct {
    struct raw_event *head;
    struct raw_event *tail;
} raw_event_queue;

typedef struct {
   int type; 
   user *target;
   user *source;
   void *target_object;
   char *created_at;
}event;

pthread_mutex_t event_buffer_mutex;
raw_event_queue *raw_event_stream;

raw_event_queue *new_raw_event_queue();
int destroy_raw_event_queue(raw_event_queue *queue);
struct raw_event *new_raw_event(char *data);
int destroy_raw_event(struct raw_event *re);
int add_raw_event(raw_event_queue *queue, char *data);
char *extract_raw_event(raw_event_queue *queue);
int start_userstream();
int stop_userstream();

pthread_t collecting_thread;
pthread_t consuming_thread;

#endif
