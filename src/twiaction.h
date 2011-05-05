#ifndef TWIACTION_H
#define TWIACTION_H

#define TL_TYPE_HOME 0
#define TL_TYPE_MENTION 1
#define TL_TYPE_USER 2

char *get_timeline(int timeline_type, char *since_id,char *max_id,char *count);

#endif
