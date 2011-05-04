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

int parse_statuses(xmlDocPtr *doc,xmlNode *node, statuses *tl){
    int nr_statuses = 0;
    status *prev = NULL;
    while(node){
        if(node->type == XML_ELEMENT_NODE && strcmp(node->name,"status")==0){
            xmlNode *attr = node->children;

            status *st = malloc(sizeof(status));
            st->next = NULL;
            nr_statuses ++;
            if(nr_statuses == 1)
                tl->head = st;
            else prev->next = st;
            st->prev = prev;

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
                else if(!xmlStrcmp(attr->name, (const xmlChar *)"user")){ //user
                    xmlNode *user_attr = attr->children;
                    while(user_attr){
                        if(!xmlStrcmp(user_attr->name, (const xmlChar *)"screen_name")){
                            char *screen_name = xmlNodeListGetString(*doc, user_attr->xmlChildrenNode, 1);
                            if(screen_name){
                                st->composer.screen_name = malloc(strlen(screen_name)+1); 
                                strcpy(st->composer.screen_name,screen_name);
                            }
                        }
                        user_attr = user_attr->next;
                    }
                }
                attr = attr->next;
            }
            if(st){
                //printf("%15s -- %s\n",st->composer.screen_name,st->text);
                prev = st;
            }
        }
        node = node->next;
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
        parse_statuses(&doc,root_element->children, tl);
    }
    else result = -1;

    xmlFreeDoc(doc);
    xmlCleanupParser();
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
