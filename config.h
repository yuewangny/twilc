#ifndef CONFIG_H
#define CONFIG_H

typedef struct {
    // key-secret pair to oauth
    char *key;
    char *secret;
    char *user_id;
    char *screen_name;
} clit_config;

int init_config(char key[],char secret[],char user_id[], char screen_name[], clit_config *config);

int save_config(clit_config *);

int parse_config(clit_config *);

#endif
