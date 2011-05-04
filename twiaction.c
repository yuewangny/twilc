#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "twiauth.h"


const char *home_api_base = "http://api.twitter.com/1/statuses/home_timeline.xml";

char *get_home(){
    return oauth_get(home_api_base,NULL,0);
}

