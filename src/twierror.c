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


#include "twierror.h"

/**
 * Get the error information string. 
 * Warning: This function is not thread-safe!
 */
char *get_error_string(int errorno){
    switch(errorno){
        case ERROR_NETWORK_CONNECTION:
            return "Error: cannot connect to twitter!";
        case ERROR_OAUTH_REPLY:
            return "Error in OAuth Reply. Please retry.";
        case ERROR_OAUTH_PIN_LENGTH:
            return "Error: PIN code length wrong!";
        case ERROR_CONFIG_WRONG:
            return "Error in config file. Please delete it and retry.";
        case ERROR_NO_WRITE_PRIVILEDGE:
            return "Error: No writing priviledge.";
        case ERROR_XML_PARSE:
            return "Error in parsing xml.";
        case ERROR_PAGE_START:
            return "Error: cannot find page start.";
        case ERROR_MALLOC:
            return "Error in memory allocation.";
        case ERROR_THREAD:
            return "Error in threading.";
        default:
            return "An unknown error occurred...";
    }
}
