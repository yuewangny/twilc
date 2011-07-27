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
#include <ncurses.h>
#include <stdlib.h>

#include "entity.h"

entity *newentity(){
    entity *e = malloc(sizeof(entity));
    e->text = 0;
    e->start = 0;
    e->end = 0;
    e->data = 0;
    e->next = 0;
    return e;
}

int destroy_entity(entity *et){
    if(et){
        if(et->text)
            free(et->text);
        free(et);
    }
    return 0;
}

void before_mention(WINDOW *win){
    init_pair(1, COLOR_YELLOW, COLOR_BLACK);
    wattron(win,COLOR_PAIR(1));
}

void after_mention(WINDOW *win){
    wattroff(win, COLOR_PAIR(1));
}

void before_url(WINDOW *win){
    init_pair(3, COLOR_BLUE, COLOR_BLACK);
    wattron(win,COLOR_PAIR(3));
    wattron(win,A_UNDERLINE);
}

void after_url(WINDOW *win){
    wattroff(win,COLOR_PAIR(3));
    wattroff(win,A_UNDERLINE);
}

void before_tag(WINDOW *win){
    init_pair(2, COLOR_RED, COLOR_BLACK);
    wattron(win,COLOR_PAIR(2));
}

void after_tag(WINDOW *win){
    wattroff(win, COLOR_PAIR(2));
}

void init_entity_types(){
    ENTITY_TYPE_MENTION = malloc(sizeof(entity_type));
    ENTITY_TYPE_MENTION->before_entity = before_mention;
    ENTITY_TYPE_MENTION->after_entity = after_mention;

    ENTITY_TYPE_URL = malloc(sizeof(entity_type));
    ENTITY_TYPE_URL->before_entity = before_url;
    ENTITY_TYPE_URL->after_entity = after_url;

    ENTITY_TYPE_HASHTAG = malloc(sizeof(entity_type));
    ENTITY_TYPE_HASHTAG->before_entity = before_tag;
    ENTITY_TYPE_HASHTAG->after_entity = after_tag;

    entity_types[0] = ENTITY_TYPE_MENTION;
    entity_types[1] = ENTITY_TYPE_HASHTAG;
    entity_types[2] = ENTITY_TYPE_URL;

}
