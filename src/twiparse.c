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

#include "twitter.h"

int parse_xml_file(char *filename, xmlDocPtr *docptr){
    *docptr = xmlReadFile(filename, NULL, 0);
    if(*docptr == NULL){
        fprintf(stderr,"Error: failed to parse!\n");
        return -1;
    }
    return 0;
}

int parse_user(xmlDocPtr *doc, xmlNode *node, user *usr){
    if(!doc || !node)
        return -1;

    if(!usr)
        return -1;
    xmlNode *attr = node->children;

    while(attr){ // parse status attributes
        if(!xmlStrcmp(attr->name, (const xmlChar *)"screen_name")){
            char *screen_name = xmlNodeListGetString(*doc, attr->xmlChildrenNode, 1);
            if(screen_name){
                usr->screen_name = malloc(strlen(screen_name)+1); 
                strcpy(usr->screen_name,screen_name);
            }
        }
        attr = attr->next;
    }
    return 0;
}

int parse_status(xmlDocPtr *doc, xmlNode *node, status *st){
    if(!node || !doc)
        return -1;
    if(!st)
        st = newstatus();
    xmlNode *attr = node->children;

    while(attr){ // parse status attributes
        if(!xmlStrcmp(attr->name, (const xmlChar *)"id")){ //status id
            char *id = xmlNodeListGetString(*doc, attr->xmlChildrenNode, 1);
            if(id){
                st->id = malloc(strlen(id)+1);
                strcpy(st->id,id);
            }
            else
                break;
        }
        else if(!xmlStrcmp(attr->name, (const xmlChar *)"text")){ //status text
            char *text = xmlNodeListGetString(*doc, attr->xmlChildrenNode, 1);
            if(text){
                st->text = malloc(strlen(text)+1);
                strcpy(st->text,text);
            }
        }
        else if(!xmlStrcmp(attr->name, (const xmlChar *)"favorited")){ 
            char *favorited = xmlNodeListGetString(*doc, attr->xmlChildrenNode, 1);
            if(favorited && strcmp(favorited,"true") == 0)
                SET_FAVORITED(st->extra_info);
        }
        else if(!xmlStrcmp(attr->name, (const xmlChar *)"user")){ //user
            if(!st->composer){
                st->composer = newuser();
                parse_user(doc,attr,st->composer);
            }
        }
        attr = attr->next;
    }
    return 0;
}

int parse_statuses(xmlDocPtr *doc,xmlNode *node, statuses *tl){
    if(!tl)
        return -1;

    int nr_statuses = 0;
    status *prev = NULL;
    while(node){
        if(node->type == XML_ELEMENT_NODE && !xmlStrcmp(node->name,(const xmlChar *)"status")){
            xmlNode *attr = node->children;

            status *st = newstatus();
            if(!st)
                continue;

            // insert the tweet into the list
            st->next = NULL;
            nr_statuses ++;
            if(nr_statuses == 1)
                tl->head = st;
            else prev->next = st;
            st->prev = prev;

            // Parse the tweet
            xmlNode *rtnode = NULL;
            while(attr){ // parse status attributes
                if(!xmlStrcmp(attr->name,(const xmlChar *)"retweeted_status")) // a retweeted status
                    rtnode = attr;
                else if(!xmlStrcmp(attr->name, (const xmlChar *)"user")){ //user
                    st->composer = newuser();
                    parse_user(doc,attr,st->composer);
                }
                attr = attr->next;
            }
            if(rtnode){
                st->retweeter = st->composer;
                st->composer = NULL;
                parse_status(doc,rtnode,st);
            }
            else
                parse_status(doc,node,st);

            prev = st;
        }
        node = node->next; //next tweet
    }
    tl->count = nr_statuses;
    return nr_statuses;
}

int parse_timeline(char *filename, statuses *tl){
    xmlDocPtr doc;
    int result = 0;
    if(parse_xml_file(filename,&doc) == -1)
        result = -1;

    xmlNode *root_element = xmlDocGetRootElement(doc);
    if(root_element->type==XML_ELEMENT_NODE
        && strcmp(root_element->name,"statuses")==0){
        if(parse_statuses(&doc,root_element->children, tl) < 0)
            result = -1;
    }
    else result = -1;

    xmlFreeDoc(doc);
    xmlCleanupParser();

    return result;
}

/*
int main(){
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
