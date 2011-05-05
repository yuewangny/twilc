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

