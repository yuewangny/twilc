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
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <string.h>
#include <pthread.h>
#include <wchar.h>

#include "twitter.h"
#include "twierror.h"

int parse_xml_file(char *filename, xmlDocPtr *docptr){
    *docptr = xmlReadFile(filename, NULL, 0);
    if(*docptr == NULL){
        SET_ERROR_NUMBER(ERROR_XML_PARSE);
        return -1;
    }
    return 0;
}

int parse_user(xmlDocPtr *doc, xmlNode *node, user **usrptr){
    if(!doc || !node || !usrptr)
        return -1;

    user *usr = *usrptr;
    xmlNode *attr = node->children;
    while(attr){ // parse user attributes
        if(!xmlStrcmp(attr->name, (const xmlChar *)"id")){
            char *id = xmlNodeListGetString(*doc, attr->xmlChildrenNode, 1);
            if(id){
                /* look up user by id */
                /*
                pthread_mutex_lock(&user_map_mutex);
                usr = g_hash_table_lookup(user_map,id);
                pthread_mutex_unlock(&user_map_mutex);
                */
                if(!usr){
                    /* create new user and insert into the map */
                    usr = newuser();
                    usr->id = strdup(id);
                    /*
                    pthread_mutex_lock(&user_map_mutex);
                    g_hash_table_insert(user_map,usr->id,usr);
                    pthread_mutex_unlock(&user_map_mutex);
                    */
                }
                else
                    break; // user exists in the map
            }
            else
                return -1;
        }
        else if(!xmlStrcmp(attr->name, (const xmlChar *)"screen_name")){
            char *screen_name = xmlNodeListGetString(*doc, attr->xmlChildrenNode, 1);
            if(screen_name){
                usr->screen_name = strdup(screen_name);
            }
        }
        else if(!xmlStrcmp(attr->name, (const xmlChar *)"name")){
            char *name = xmlNodeListGetString(*doc,attr->xmlChildrenNode,1);
            if(name){
                usr->name = strdup(name);
            }
        }
        else if(!xmlStrcmp(attr->name, (const xmlChar *)"location")){
            char *location = xmlNodeListGetString(*doc,attr->xmlChildrenNode,1);
            if(location){
                usr->location = strdup(location);
            }
        }
        else if(!xmlStrcmp(attr->name, (const xmlChar *)"description")){
            char *description = xmlNodeListGetString(*doc,attr->xmlChildrenNode,1);
            if(description){
                usr->bio = strdup(description);
            }
        }
        else if(!xmlStrcmp(attr->name, (const xmlChar *)"url")){
            char *url = xmlNodeListGetString(*doc,attr->xmlChildrenNode,1);
            if(url){
                usr->url = strdup(url);
            }
        }
        attr = attr->next;
    }
    *usrptr = usr;
    return 0;
}

int parse_mention_entity(xmlDocPtr *doc, xmlNode *node, entity **ent, status *st){
    user *usr = NULL;
    xmlNode *mention_attr = node->children;


    while(mention_attr){
        if(!xmlStrcmp(mention_attr->name,(const xmlChar *)"id")){
            char *id = xmlNodeListGetString(*doc, mention_attr->xmlChildrenNode, 1);
            if(!id)
                break;
            /*
            pthread_mutex_lock(&user_map_mutex);
            usr = g_hash_table_lookup(user_map,id);
            pthread_mutex_unlock(&user_map_mutex);
            */
            if(!usr){
                usr = newuser();
                usr->id = strdup(id);
                /*
                pthread_mutex_lock(&user_map_mutex);
                g_hash_table_insert(user_map,usr->id,usr);
                pthread_mutex_unlock(&user_map_mutex);
                */
            }
        }
        else if(!xmlStrcmp(mention_attr->name,(const xmlChar *)"screen_name")){
            if(!usr)
                break;
            if(!(usr->screen_name)){
                char *screen_name = xmlNodeListGetString(*doc, mention_attr->xmlChildrenNode, 1);
                usr->screen_name = strdup(screen_name);
            }
        }
        else if(!xmlStrcmp(mention_attr->name,(const xmlChar *)"name")){
            if(!usr)
                break;
            if(!(usr->name)){
                char *name = xmlNodeListGetString(*doc, mention_attr->xmlChildrenNode, 1);
                usr->name = strdup(name);
            }
        }

        mention_attr = mention_attr->next;
    }

    if(!usr)
        return -1;

    *ent = newentity();
    (*ent)->type = ENTITY_TYPE_MENTION;
    (*ent)->data = usr;
    int start = atoi((char *)xmlGetProp(node,"start"));
    int end = atoi((char *)xmlGetProp(node,"end"));
    (*ent)->text = malloc((end-start+1)*sizeof(wchar_t));
    memcpy((*ent)->text,st->wtext+start,(end-start)*sizeof(wchar_t));
    (*ent)->text[end-start] = '\0';
    (*ent)->start = start;
    (*ent)->end = end;
    wprintf(L"%ls\n",(*ent)->text);
    return 0;
}

int parse_url_entity(xmlDocPtr *doc, xmlNode *node, entity **ent, status *st){
    xmlNode *url_attr = node->children;
    char *url = NULL;

    while(url_attr){
        if(!xmlStrcmp(url_attr->name,(const xmlChar *)"url")){
            url = xmlNodeListGetString(*doc, url_attr->xmlChildrenNode, 1);
        }
        url_attr = url_attr->next;
    }

    if(!url)
        return -1;
    *ent = newentity();
    (*ent)->type = ENTITY_TYPE_URL;
    (*ent)->data = NULL;
    int start = atoi((char *)xmlGetProp(node,"start"));
    int end = atoi((char *)xmlGetProp(node,"end"));
    (*ent)->text = malloc((end-start+1)*sizeof(wchar_t));
    memcpy((*ent)->text,st->wtext+start,(end-start)*sizeof(wchar_t));
    (*ent)->text[end-start] = '\0';
    (*ent)->start = start;
    (*ent)->end = end;
    return 0;
}

int parse_hashtag_entity(xmlDocPtr *doc, xmlNode *node, entity **ent, status *st){
    xmlNode *hashtag_attr = node->children;

    while(hashtag_attr){
        if(!xmlStrcmp(hashtag_attr->name,(const xmlChar *)"text")){
            char *hashtagstr = xmlNodeListGetString(*doc, hashtag_attr->xmlChildrenNode, 1);
        }
        hashtag_attr = hashtag_attr->next;
    }

    *ent = newentity();
    (*ent)->type = ENTITY_TYPE_HASHTAG;
    (*ent)->data = NULL;
    int start = atoi((char *)xmlGetProp(node,"start"));
    int end = atoi((char *)xmlGetProp(node,"end"));
    printf("start:%d,end:%d\n",start,end);
    (*ent)->text = malloc((end-start+1)*sizeof(wchar_t));
    memcpy((*ent)->text,st->wtext+start,(end-start)*sizeof(wchar_t));
    (*ent)->text[end-start] = '\0';
    (*ent)->start = start;
    (*ent)->end = end;
    return 0;
}

int parse_entities(xmlDocPtr *doc, xmlNode *node, status *st){
    xmlNode *ent_node = node->children;
    entity **entt_ptr = &(st->entities);
    st->entity_count = 0;
    while(ent_node){
        if(!xmlStrcmp(ent_node->name,(const xmlChar *)"user_mentions")){
            xmlNode *user_mention = ent_node->children;
            while(user_mention){
                if(!xmlStrcmp(user_mention->name,(const xmlChar *)"user_mention")){
                    parse_mention_entity(doc,user_mention,entt_ptr,st);
                    if(*entt_ptr){
                        st->entity_count ++;
                        entt_ptr = &((*entt_ptr)->next);
                    }
                }
                user_mention = user_mention->next;
            }
        }
        else if(!xmlStrcmp(ent_node->name,(const xmlChar *)"urls")){
            xmlNode *url_node = ent_node->children;
            while(url_node){
                if(!xmlStrcmp(url_node->name,(const xmlChar *)"url")){
                    parse_url_entity(doc,url_node,entt_ptr,st);
                    if(*entt_ptr){
                        st->entity_count ++;
                        entt_ptr = &((*entt_ptr)->next);
                    }
                }
                url_node = url_node->next;
            }
        }
        else if(!xmlStrcmp(ent_node->name,(const xmlChar *)"hashtags")){
            xmlNode *hashtag_node = ent_node->children;
            while(hashtag_node){
                if(!xmlStrcmp(hashtag_node->name,(const xmlChar *)"hashtag")){
                    parse_hashtag_entity(doc,hashtag_node,entt_ptr,st);
                    if(*entt_ptr){
                        st->entity_count ++;
                        entt_ptr = &((*entt_ptr)->next);
                    }
                }
                hashtag_node = hashtag_node->next;
            }
        }
        ent_node = ent_node->next;
    }
}


int parse_status(xmlDocPtr *doc, xmlNode *node, status **stptr){
    if(!node || !doc || !stptr)
        return -1;
    xmlNode *attr = node->children;

    status *st = *stptr;
    while(attr){ // parse status attributes
        if(!xmlStrcmp(attr->name, (const xmlChar *)"id")){ //status id
            char *id = xmlNodeListGetString(*doc, attr->xmlChildrenNode, 1);
            if(id){
                // look up status by id
                /*
                pthread_mutex_lock(&status_map_mutex);
                st = g_hash_table_lookup(status_map,id);
                pthread_mutex_unlock(&status_map_mutex);
                */
                if(!st){
                    // create new status and insert into the map
                    st = newstatus();
                    st->id = strdup(id);
                    /*
                    pthread_mutex_lock(&status_map_mutex);
                    g_hash_table_insert(status_map,st->id,st);
                    pthread_mutex_unlock(&status_map_mutex);
                    */
                }
                else
                    break;
            }
            else
                break;
        }
        else if(!xmlStrcmp(attr->name, (const xmlChar *)"text")){ //status text
            char *text = xmlNodeListGetString(*doc, attr->xmlChildrenNode, 1);
            if(text){
                st->wtext = malloc((TWEET_MAX_LEN+1)*sizeof(wchar_t));
                memset(st->wtext,'\0',(TWEET_MAX_LEN+1)*sizeof(wchar_t));
                st->length = mbstowcs(st->wtext,text,TWEET_MAX_LEN);
            }
        }
        else if(!xmlStrcmp(attr->name, (const xmlChar *)"favorited")){ 
            char *favorited = xmlNodeListGetString(*doc, attr->xmlChildrenNode, 1);
            if(favorited && strcmp(favorited,"true") == 0)
                SET_FAVORITED(st->extra_info);
        }
        else if(!xmlStrcmp(attr->name, (const xmlChar *)"user")){ //user
            parse_user(doc,attr,&(st->composer));
        }
        /*
        else if(!xmlStrcmp(attr->name, (const xmlChar *)"in_reply_to_status_id")){ 
            char *in_reply_to_status_id = xmlNodeListGetString(*doc, attr->xmlChildrenNode, 1);
            if(in_reply_to_status_id && strlen(in_reply_to_status_id) > 0){
                st->in_reply_to_status_id = strdup(in_reply_to_status_id);
            }
        }
        */
        else if(!xmlStrcmp(attr->name,(const xmlChar *)"retweeted_status")){
            st->retweeted_status = newstatus();
            parse_status(doc,attr,&(st->retweeted_status));
        }
        else if(!xmlStrcmp(attr->name,(const xmlChar *)"entities")){
            //printf("status:%s\n",st->text);
            parse_entities(doc,attr,st);
        }
        attr = attr->next;
    }
    split_status_entities(st);
    *stptr = st;
    return 0;
}

int parse_statuses(xmlDocPtr *doc,xmlNode *node, statuses *tl){
    if(!tl)
        return -1;

    int nr_statuses = 0;
    struct status_node *prev = NULL;
    node = node->children;
    while(node){
        if(node->type == XML_ELEMENT_NODE && !xmlStrcmp(node->name,(const xmlChar *)"status")){
            xmlNode *attr = node->children;

            struct status_node *sn = newstatusnode(NULL);
            if(!sn)
                continue;

            // insert the tweet into the list
            sn->next = NULL;
            nr_statuses ++;
            if(nr_statuses == 1)
                tl->head = sn;
            else prev->next = sn;
            sn->prev = prev;
            parse_status(doc,node,&(sn->st));
            prev = sn;
        }
        node = node->next;
    }
    tl->count = nr_statuses;
    return nr_statuses;
}

/*
int parse_error(xmlDocPtr *doc,xmlNode *node){
}
*/

int parse_timeline(char *filename, statuses *tl){
    xmlDocPtr doc;
    int result = 0;
    if(parse_xml_file(filename,&doc) == -1)
        result = -1;

    xmlNode *root_element = xmlDocGetRootElement(doc);
    if(root_element->type==XML_ELEMENT_NODE
        && strcmp(root_element->name,"statuses")==0){
        if(parse_statuses(&doc,root_element, tl) < 0)
            result = -1;
    }
    else{
        result = -1;
        /*
        parse_error(&doc,root_element);
        */
    } 

    xmlFreeDoc(doc);
    xmlCleanupParser();

    return result;
}

status *parse_single_status(char *filename){
    xmlDocPtr doc;
    int result = 0;
    if(parse_xml_file(filename,&doc) == -1)
        return NULL;

    xmlNode *root_element = xmlDocGetRootElement(doc);
    status *st = NULL;
    if(root_element->type==XML_ELEMENT_NODE
        && strcmp(root_element->name,"status")==0){
        parse_status(&doc,root_element, &st);
    }
    /*
    else
        parse_error(doc,root_element);
        */

    xmlFreeDoc(doc);
    xmlCleanupParser();

    return st;
}

/*
int test_xml_parse(){
    LIBXML_TEST_VERSION
    statuses tl;
    parse_timeline("clit.tmp",&tl); 
    
    status *p = tl.head;
    while(p){
        printf("%15s--%s\n", p->composer.screen_name, p->text);
        p = p->next;
    }
}
*/
