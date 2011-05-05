#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <oauth.h>
#include <curl/curl.h>

#include "twiauth.h"

#define TEMP_FILE "clit.tmp"

const char *REQUEST_TOKEN_URL = "https://api.twitter.com/oauth/request_token";
const char *ACCESS_TOKEN_URL = "https://api.twitter.com/oauth/access_token";
const char *AUTHORIZE_URL = "https://api.twitter.com/oauth/authorize";
const char *CONSUMER_KEY = "bRHhkPtSm0LINx323EiWCA";
const char *CONSUMER_SECRET = "OH2xUyk8CH1xdI1ec6fkxDT5oImfDZKvlnqZIogaOM";

const char *ACCESS_TOKEN;
const char *ACCESS_TOKEN_SECRET;

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
    //printf("request URL:%s\n",req_url);
    reply = oauth_http_post(req_url, postarg);
    if(req_url) free(req_url);
    if(postarg) free(postarg);
    if(!reply)
        return 1;
    //else
        //printf("http-reply:%s\n",reply);

    rc = oauth_split_url_parameters(reply, &rv);
    qsort(rv, rc, sizeof(char *), oauth_cmpstringp);
    //printf("%d parameters:%s,%s,%s\n",rc,rv[0],rv[1],rv[2]);
    if(rc != 3 || strncmp(rv[0],"oauth_callback_confirmed=true",29) || strncmp(rv[1], "oauth_token=",11) || strncmp(rv[2],"oauth_token_secret=",18)){
        free(rv);
        return 1;
    }

    oauth_token = strdup(rv[1]+12);
    oauth_token_secret = strdup(rv[2]+19);
    free(rv);
    free(reply);
    if(!oauth_token || !oauth_token_secret)
        return 1;
    
    printf("Please authorize: %s?oauth_token=%s\n",AUTHORIZE_URL,oauth_token);
    oauth_verifier = (char *)malloc(8*sizeof(char));
    printf("Enter PIN code:");
    scanf("%s",oauth_verifier);
    if(strlen(oauth_verifier) != 7){
        free(oauth_verifier);
        return 1;
    }
    
    postarg = NULL; 
    req_url = oauth_sign_url2(ACCESS_TOKEN_URL,&postarg,OA_HMAC,NULL,CONSUMER_KEY, CONSUMER_SECRET,oauth_token,oauth_token_secret);
    postarg_verified = (char *)malloc((strlen(postarg)+24)*sizeof(char));
    sprintf(postarg_verified,"%s&oauth_verifier=%s", postarg,oauth_verifier);
    //printf("request URL:%s\n",postarg_verified);
    reply = oauth_http_post(req_url, postarg_verified);

    free(req_url);
    free(postarg);
    if(reply)
        //oauth_token;oauth_token_secret;user_id;screen_name
        printf("reply:%s\n",reply);
    else
        return 1;

    rv = NULL;
    rc = oauth_split_url_parameters(reply, &rv);
    qsort(rv, rc, sizeof(char *), oauth_cmpstringp);
    printf("%d parameters:%s,%s,%s,%s\n",rc,rv[0],rv[1],rv[2],rv[3]);

    if(rc !=4 || strncmp(rv[0],"oauth_token=",11) || strncmp(rv[1],"oauth_token_secret=",18) || strncmp(rv[2],"screen_name=",11) || strncmp(rv[3],"user_id=",7)){
        free(rv);
        return 1;
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
    if(key == NULL || secret == NULL)
        return -1;
    ACCESS_TOKEN = key;
    ACCESS_TOKEN_SECRET = secret;
    return 0;
}

char *http_get(char *url, char *param){
    char *fullurl = malloc(strlen(url)+strlen(param)+2);
    strcpy(fullurl,url);
    strcat(fullurl,"?");
    strcat(fullurl,param);

    FILE *fp = NULL;
    curl_global_init(CURL_GLOBAL_SSL);
    CURL *handle = curl_easy_init();
    if(handle){
        curl_easy_setopt(handle,CURLOPT_URL,fullurl);
        fp = fopen(TEMP_FILE,"w+");
        if(!fp)
            fprintf(stderr,"Error opening tmp file!\n");
        else{
            //printf("==============================\n");
            curl_easy_setopt(handle,CURLOPT_WRITEDATA,fp);
            CURLcode res = curl_easy_perform(handle);
            curl_easy_cleanup(handle);
            fclose(fp);
            //printf("=======================================\n");
        }
    }

    free(fullurl);
    return TEMP_FILE;
}

char *oauth_get(const char *api_base, kvpair *params, int nr_param){
    char *arg;
    char *fullarg;
    char *req_url;
    char *reply;

    arg = NULL; 
    req_url = oauth_sign_url2(api_base,&arg,OA_HMAC,"GET",CONSUMER_KEY, CONSUMER_SECRET,ACCESS_TOKEN,ACCESS_TOKEN_SECRET);

    int arglen = strlen(arg);
    int i;
    for(i = 0; i < nr_param; i++)
        arglen += strlen(params[i].key)+strlen(params[i].value+2);
    arglen ++;
    fullarg = (char *)malloc(arglen);
    memset(fullarg,'\0',arglen);
    strcpy(fullarg,arg);
    for(i=0; i < nr_param; i++){
        strcat(fullarg,"&");
        strcat(fullarg,params[i].key);
        strcat(fullarg,"=");
        strcat(fullarg,params[i].value);
    }
    //reply = oauth_http_get(req_url, fullarg);
    char *fn = http_get(req_url,fullarg);

    free(fullarg);
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


