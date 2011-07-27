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
#include <oauth.h>
#include <curl/curl.h>
#include <pthread.h>

#include "twiauth.h"
#include "twierror.h"

#define TEMP_FILE "clit.tmp"

const char *REQUEST_TOKEN_URL = "https://api.twitter.com/oauth/request_token";
const char *ACCESS_TOKEN_URL = "https://api.twitter.com/oauth/access_token";
const char *AUTHORIZE_URL = "https://api.twitter.com/oauth/authorize";
const char *CONSUMER_KEY = "bRHhkPtSm0LINx323EiWCA";
const char *CONSUMER_SECRET = "OH2xUyk8CH1xdI1ec6fkxDT5oImfDZKvlnqZIogaOM";


int oauth_authorize(char **access_token, char **access_token_secret, char **user_id, char **screen_name){
    char *req_url = NULL;
    char *reply = NULL;
    char *postarg = NULL;
    int rc;
    char **rv = NULL;
    
    char *oauth_token = NULL;
    char *oauth_token_secret = NULL;
    char *oauth_verifier;
    char *postarg_verified = NULL;

    req_url = oauth_sign_url2(REQUEST_TOKEN_URL, &postarg, OA_HMAC, NULL, CONSUMER_KEY, CONSUMER_SECRET, NULL, NULL);
    reply = oauth_http_post(req_url, postarg);
    if(req_url) free(req_url);
    if(postarg) free(postarg);
    if(!reply){
        error_no = ERROR_NETWORK_CONNECTION;
        return -1;
    }

    rc = oauth_split_url_parameters(reply, &rv);
    qsort(rv, rc, sizeof(char *), oauth_cmpstringp);
    if(rc != 3 || strncmp(rv[0],"oauth_callback_confirmed=true",29) || strncmp(rv[1], "oauth_token=",11) || strncmp(rv[2],"oauth_token_secret=",18)){
        free(rv);
        error_no = ERROR_OAUTH_REPLY;
        return -1;
    }

    oauth_token = strdup(rv[1]+12);
    oauth_token_secret = strdup(rv[2]+19);
    free(rv);
    free(reply);
    if(!oauth_token || !oauth_token_secret){
        error_no = UNKNOWN_ERROR;
        return -1;
    }
    
    printf("Please authorize: %s?oauth_token=%s\n",AUTHORIZE_URL,oauth_token);
    oauth_verifier = (char *)malloc(8*sizeof(char));
    printf("Enter PIN code:");
    scanf("%s",oauth_verifier);
    if(strlen(oauth_verifier) != 7){
        free(oauth_verifier);
        error_no = ERROR_OAUTH_PIN_LENGTH;
        return -1;
    }
    
    postarg = NULL; 
    req_url = oauth_sign_url2(ACCESS_TOKEN_URL,&postarg,OA_HMAC,NULL,CONSUMER_KEY, CONSUMER_SECRET,oauth_token,oauth_token_secret);
    postarg_verified = (char *)malloc((strlen(postarg)+24)*sizeof(char));
    sprintf(postarg_verified,"%s&oauth_verifier=%s", postarg,oauth_verifier);
    reply = oauth_http_post(req_url, postarg_verified);

    free(req_url);
    free(postarg);
    if(!reply){
        error_no = ERROR_NETWORK_CONNECTION;
        return -1;
    }

    rv = NULL;
    rc = oauth_split_url_parameters(reply, &rv);
    qsort(rv, rc, sizeof(char *), oauth_cmpstringp);
    printf("%d parameters:%s,%s,%s,%s\n",rc,rv[0],rv[1],rv[2],rv[3]);

    if(rc !=4 || strncmp(rv[0],"oauth_token=",11) || strncmp(rv[1],"oauth_token_secret=",18) || strncmp(rv[2],"screen_name=",11) || strncmp(rv[3],"user_id=",7)){
        free(rv);
        error_no = ERROR_OAUTH_REPLY;
        return -1;
    }
    *access_token = (char *)malloc((strlen(rv[0])-11)*sizeof(char));
    *access_token_secret = (char *)malloc((strlen(rv[1]-18))*sizeof(char));
    *screen_name = (char *)malloc((strlen(rv[2])-11)*sizeof(char));
    *user_id = (char *)malloc((strlen(rv[3])-7)*sizeof(char));

    strcpy(*access_token,rv[0]+12);
    strcpy(*access_token_secret,rv[1]+19);
    strcpy(*screen_name,rv[2]+12);
    strcpy(*user_id,rv[3]+8);

    free(reply);
    free(rv);

    return 0;
}

int init_oauth(char *key, char *secret){
    if(key == NULL || secret == NULL){
        error_no = ERROR_CONFIG_WRONG;
        return -1;
    }
    ACCESS_TOKEN = key;
    ACCESS_TOKEN_SECRET = secret;
    return 0;
}

char *http_get(char *url){
    FILE *fp = NULL;
    curl_global_init(CURL_GLOBAL_SSL);
    CURL *handle = curl_easy_init();
    if(handle){
        curl_easy_setopt(handle,CURLOPT_URL,url);
        fp = fopen(TEMP_FILE,"w+");
        if(!fp){
            SET_ERROR_NUMBER(ERROR_NO_WRITE_PRIVILEDGE);
            return NULL;
        }
        else{
            curl_easy_setopt(handle,CURLOPT_WRITEDATA,fp);
            CURLcode res = curl_easy_perform(handle);
            curl_easy_cleanup(handle);
            fclose(fp);
        }
    }
    else{
        SET_ERROR_NUMBER(ERROR_NETWORK_CONNECTION);
        return NULL;
    }

    return TEMP_FILE;
}

char *oauth_get(const char *api_base, kvpair *params, int nr_param, int get_or_post){
    char *call_url = 0;
    char *req_url = 0;

    int len = strlen(api_base);
    for(int i = 0; i < nr_param; i++)
        len += strlen(params[i].key)+strlen(params[i].value)+2;
    len ++;
    call_url = (char *)malloc(len+1);

    memset(call_url,'\0',len);
    strcpy(call_url,api_base);
    for(int i=0; i < nr_param; i++){
        if(i == 0)
            strcat(call_url,"?");
        else
            strcat(call_url,"&");
        strcat(call_url,params[i].key);
        strcat(call_url,"=");
        strcat(call_url,params[i].value);
    }

    char *fn = NULL;
    if(get_or_post == GET){
        req_url = oauth_sign_url2(call_url ,NULL,OA_HMAC,NULL,CONSUMER_KEY, CONSUMER_SECRET,ACCESS_TOKEN,ACCESS_TOKEN_SECRET);
        //char *fn = oauth_http_get(req_url, arg);
        fn = http_get(req_url);
    }
    else{
        char *postarg;
        req_url = oauth_sign_url2(call_url ,&postarg,OA_HMAC,NULL,CONSUMER_KEY, CONSUMER_SECRET,ACCESS_TOKEN,ACCESS_TOKEN_SECRET);
        oauth_http_post(req_url,postarg);
    }

    free(call_url);
    free(req_url);

    return fn;
}

void test_authorize(){
    char *access_token;
    char *access_token_secret;
    char *user_id;
    char *screen_name;

    if(oauth_authorize(&access_token,&access_token_secret,&user_id,&screen_name))
        printf("failed\n");
    else
        printf("access_token:%s\naccess_token_secret:%s\nscreen_name:%s\nuser_id:%s\n",access_token,access_token_secret,screen_name,user_id);
}

/*
int main(){
    int nr_param = 0;
    char *api_base = "http://api.twitter.com/1/statuses/home_timeline.xml";
    init_oauth("15975251-xuyUN5j1flGOphScil8sJhLD4mr17hgRWQr1HNTSb","eksfLlsNmj9F7SFbsx2qQdCc8ELfkRVtTkyAG0oi7tI");
    oauth_get(api_base,NULL,0);
}
*/


