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


#ifndef ENTITY_H 
#define ENTITY_H 

#define ENTITY_TYPE_NUM 3 

#include <stdint.h>
#include <ncurses.h>
#include <wchar.h>

typedef struct {
    void (*before_entity)(WINDOW *);
    void (*after_entity)(WINDOW *);
    void (*entity_action)(void);
} entity_type;

// entity
typedef struct entity_struct{
    entity_type *type;
    wchar_t *text;
    uint8_t start;
    uint8_t end;
    void *data;

    struct entity_struct *next;
} entity;

entity_type *ENTITY_TYPE_MENTION;
entity_type *ENTITY_TYPE_URL;
entity_type *ENTITY_TYPE_HASHTAG;

void init_entity_types();
int destroy_entity(entity *);

#endif
