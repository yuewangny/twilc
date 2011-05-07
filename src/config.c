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

#include "config.h"

#define CONFIG_FILE_NAME ".clitcfg"

int init_config(char key[],char secret[], char user_id[], char screen_name[],clit_config *config){
    if(!config)
        config = malloc(sizeof(clit_config));
    if(config == NULL){
        printf("No memory!\n");
        return -1;
    }

    config->key = key;
    config->secret = secret;
    config->user_id = user_id;
    config->screen_name = screen_name;

    return 0;
}

char *get_config_path(){
    char *home = getenv("HOME");
    char *config_file_path = malloc(strlen(home)+strlen(CONFIG_FILE_NAME)+2);
    int temp = strlen(home);
    if(home[temp-1] == '/')
        temp -= 1;
    memcpy(config_file_path,home,temp);
    config_file_path[temp] = '/';
    strcpy(config_file_path+temp+1,CONFIG_FILE_NAME);

    return config_file_path;
}

int save_config(clit_config *config){
    FILE *fp = fopen(get_config_path(),"w+");
    if(!fp){
        fprintf(stderr,"Error:cannot open config file!\n");
        return -1;
    }
    fprintf(fp,"key = %s\n",config->key);
    fprintf(fp,"secret = %s\n", config->secret);
    fprintf(fp,"user_id = %s\n", config->user_id);
    fprintf(fp,"screen_name = %s\n", config->screen_name);
    fclose(fp);

    return 0;
}

char *parse_value(char *line){
    int pos1 = 0,pos2=0;
    char *value;

    while(line[pos1++] != '=');
    while(line[pos1] == ' ') pos1++;
    pos2 = pos1;
    while(line[pos2] !=' ' && line[pos2] != '\n')
        pos2++;
    value = malloc(pos2-pos1+1);
    if(!value){
        fprintf(stderr,"Not enough memory!\n");
        return NULL;
    }
    strncpy(value,line+pos1,pos2-pos1);
    value[pos2-pos1] = '\0';

    return value;
}

int parse_config(clit_config *config){
    FILE *fp = fopen(get_config_path(),"r");
    if(!fp){
        //fprintf(stderr,"Error:cannot open config file!\n");
        return -1;
    }
    if(config == NULL)
        config = malloc(sizeof(clit_config));
    char line[100];

    memset(line,100,'\0');
    fgets(line,100,fp);
    config->key = parse_value(line);

    memset(line,100,'\0');
    fgets(line,100,fp);
    config->secret = parse_value(line);

    memset(line,100,'\0');
    fgets(line,100,fp);
    config->user_id = parse_value(line);

    memset(line,100,'\0');
    fgets(line,100,fp);
    config->screen_name = parse_value(line);

    if(config->key == NULL || config->secret == NULL || config->user_id == NULL || config->screen_name == NULL)
        return -1;

    return 0;
}

int test_save(){
    clit_config *config = malloc(sizeof(clit_config));
    init_config("15975251-xuyUN5j1flGOphScil8sJhLD4mr17hgRWQr1HNTSb","eksfLlsNmj9F7SFbsx2qQdCc8ELfkRVtTkyAG0oi7tI","15975251","pipitu",config);
    save_config(config);
}

int test_parse(){
    clit_config *config = malloc(sizeof(clit_config));
    parse_config(config);
}

