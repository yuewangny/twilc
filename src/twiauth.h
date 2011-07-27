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


#ifndef TWIAUTH_H
#define TWIAUTH_H


typedef struct{
    char *key;
    char *value;
} kvpair;

#define GET 0
#define POST 1

const char *ACCESS_TOKEN;
const char *ACCESS_TOKEN_SECRET;

int oauth_authorize(char **access_token, char **access_token_secret, char **user_id, char **screen_name);

int init_oauth(char *,char *);

char *oauth_get(const char *api_base, kvpair *params, int nr_param, int get_or_post);

#endif
