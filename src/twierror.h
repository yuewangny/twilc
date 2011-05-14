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


#ifndef TWIERROR_H
#define TWIERROR_H

#include <stdint.h>
#include <pthread.h>

int error_no;
pthread_mutex_t error_mutex;

#define ERROR_NETWORK_CONNECTION 0
#define ERROR_OAUTH_REPLY 1
#define ERROR_OAUTH_PIN_LENGTH 2
#define ERROR_CONFIG_WRONG 3
#define ERROR_NO_WRITE_PRIVILEDGE 4
#define ERROR_XML_PARSE 5
#define ERROR_PAGE_START 6
#define ERROR_MALLOC 7
#define ERROR_THREAD 8

#define UNKNOWN_ERROR 255

#define SET_ERROR_NUMBER(number) \
    while(pthread_mutex_lock(&error_mutex)); \
    error_no = number; \
    pthread_mutex_unlock(&error_mutex);


char *get_error_string(int error_no);

#endif
