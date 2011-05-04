#ifndef TWIAUTH_H
#define TWIAUTH_H


typedef struct{
    char *key;
    char *value;
} kvpair;

int oauth_authorize(char **access_token, char **access_token_secret, char **user_id, char **screen_name);

int init_oauth(char *,char *);

char *oauth_get(const char *api_base, kvpair *params, int nr_param);

#endif
