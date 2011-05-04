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

#define TWEET_STATE_DELETED 0
#define TWEET_TO_LOAD 0

int init_timelines(){
    for(int i = 0; i < TIMELINE_COUNT; ++i){
        timelines[i] = malloc(sizeof(statuses));
        timelines[i]->count = 0;
    }
    statuses *home = timelines[0];
    char *tmpfile = get_home();
    load_timeline(tmpfile,home);
    for(status *s = home->head; s; s = s->next){
        printf("%s\n",s->text);
        filter_status_text(s);
    }
    current_status[0] = timelines[0]->head;
    current_top_status[0] = timelines[0]->head;

    current_tl_index = 0;

    return 0;
}

int destroy_timeline(statuses *tl){
    status *p = tl->head;
    while(p){
        destroy_status(p);
        p = p->next;
    }
    free(tl);
    return 0;
}


int destroy_status(status *s){
    if(!s)
        return 0;
    if(s->id){
        free(s->id);
        free(s->composer.id);
        free(s->composer.screen_name);
        for(int i=0;i<s->filter_count;i++)
            free(s->filtered_text[i]);
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
        //printf("access_token:%s\naccess_token_secret:%s\nscreen_name:%s\nuser_id:%s\n",access_token,access_token_secret,screen_name,user_id);
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
            printf("%s\n",filtered_text[i]);
            i++;
        }

        len = end - begin;
        filtered_text[i] = malloc((len + 1)*sizeof(char));
        strncpy(filtered_text[i],begin,len);
        filtered_text[i][len] = '\0';
        filter_list[i] = current_filter;
        printf("%s\n",filtered_text[i]);
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
        printf("%s\n",filtered_text[i]);
        i ++;
    }
    s->filter_count = i;
}

/*
 * Merge new tweets with current timeline.
 */
int load_timeline(char *tmpfile, statuses *tl){
    LIBXML_TEST_VERSION
    statuses *toptweets = malloc(sizeof(statuses));
    toptweets->count = 0;
    parse_timeline(tmpfile,toptweets);

    if(tl->count == 0){ // No old tweets
        tl->head = toptweets->head;
        tl->count = toptweets->count;
        return tl->count;
    }
    else{
        status *top = toptweets->head;
        status *oldtop = tl->head;
        if(strcmp(top->id,oldtop->id) == 0)
            return 0;

        status *prev = top;
        while(top && oldtop){
            int result = strcmp(top->id,oldtop->id);
            if(result > 0){ // new tweet
                prev = top;
                top = top->next;
            }
            else if(result < 0){ // old tweet already deleted
                status *tmp = oldtop;
                oldtop = oldtop->next;
                destroy_status(tmp);
            }
            else{
                prev->next = oldtop;
                oldtop->prev = prev;
                break;
            }
        }

        // Gap between new and old tweets
        if(!top){
            status *temp = malloc(sizeof(status));
            temp->id = TWEET_TO_LOAD;
            temp->text = "\nGap\n";
            prev->next = temp;
            temp->prev = prev;
            temp->next = oldtop;
            oldtop->prev = temp;
        }

        // free deleted tweets
        status *p = tl->head;
        while(p && p != oldtop){
            prev = p;
            p = p->next;
            free(prev);
        }

        tl->head = toptweets->head;
    }
}


