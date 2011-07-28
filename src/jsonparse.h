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

#ifndef JSONPARSE_H
#define JSONPARSE_H

#include "twitter.h"
#include "streaming.h"
#include <jansson.h>

int parse_friends_json(json_t *friends_root, void *data_ptr);
int parse_user_json(json_t *user_root, user **user_ptr);
int parse_status_json(json_t *status_root, status **status_ptr);
int parse_event_json(json_t *event_root, event **event_ptr);

#endif

