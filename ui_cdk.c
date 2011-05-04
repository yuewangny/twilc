#include <ncurses.h>
#include <cdk/cdk.h>
#include <stdlib.h>
#include <stdio.h>
#include <locale.h>
#include <libxml/parser.h>
#include <string.h>

#include "twitter.h"
#include "config.h"
#include "twiauth.h"
#include "twiaction.h"
#include "filter.h"

#define INPUT_MAX_SIZE 281
#define INPUT_WIN_HEIGHT 8
#define INPUT_WIN_WIDTH 60

#define TWEET_STATE_DELETED 0
#define TWEET_TO_LOAD 0

int parse_timeline(char *filename, statuses *tl);

int load_timeline(char *tmpfile, statuses *tl){
    LIBXML_TEST_VERSION
    statuses *toptweets = malloc(sizeof(statuses));
    toptweets->count = 0;
    parse_timeline(tmpfile,toptweets);

    if(tl->count == 0){
        tl->head = toptweets->head;
        tl->count = toptweets->count;
        return tl->count;
    }
    else{
        status *top = toptweets->head;
        status *oldtop = tl->head;
        if(strcmp(top->id,oldtop->id) == 0)
            return 0;

        status *prev = top;
        while(top && oldtop){
            int result = strcmp(top->id,oldtop->id);
            if(result > 0){
                prev = top;
                top = top->next;
            }
            else if(result < 0){
                oldtop->id = TWEET_STATE_DELETED;
                oldtop = oldtop->next;
            }
            else{
                prev->next = oldtop;
                break;
            }
        }

        // Gap between new and old tweets
        if(!top){
            status *temp = malloc(sizeof(status));
            temp->id = TWEET_TO_LOAD;
            temp->text = "... ...";
            prev->next = temp;
            temp->next = oldtop;
        }

        // free deleted tweets
        status *p = tl->head;
        while(p != oldtop){
            prev = p;
            p = p->next;
            free(prev);
        }

        tl->head = toptweets->head;
    }
}

int main(int argc, char *argv[]){
    /* Read config file */
    clit_config *config = malloc(sizeof(clit_config));
    parse_config(config);
    printf("key:%s,secret:%s\n",config->key,config->secret);
    /* If no user info exists, intialize oauth process */
    init_oauth(config->key,config->secret);

    create_filters();

    home = malloc(sizeof(statuses));
    home->count = 0;

    setlocale(LC_ALL,"");
    CDKSCREEN *cdkscreen = 0;
    CDKSCROLL *home_scroll = 0;
    WINDOW *cursesWin = 0;
    char *title = "<C><\5>Home";
    char **home_items = 0;
    int selection,count;

    char *tmpfile = get_home();
    count = load_timeline(tmpfile,home);
    printf("count:%d\n",count);
    home_items = malloc(count*sizeof(char *));

    CDK_PARAMS params;
    CDKparseParams (argc, argv, &params, "cs:t:" CDK_CLI_PARAMS);

    cursesWin = initscr();
    cdkscreen = initCDKScreen(cursesWin);
    initCDKColor();

    status *p = home->head;
    for(int i=0;i<count;++i){
        if(p){
            home_items[i] = p->text;
            p = p->next;
        }
        else break;
    }
//    count = CDKgetDirectoryContents (".", &home_items);

    home_scroll = newCDKScroll(cdkscreen,
                               CDKparamValue (&params, 'X', CENTER),
                               CDKparamValue (&params, 'Y', CENTER),
                               CDKparsePosition(CDKparamString2(&params, 's', "RIGHT")),
                               CDKparamValue (&params, 'H', 0),
                               CDKparamValue (&params, 'W', 0),
                               CDKparamString2(&params, 't', title),
                               CDKparamNumber(&params, 'c') ? 0 : home_items,
                               CDKparamNumber(&params, 'c') ? 0 : count,
                               TRUE,
                               A_REVERSE,
                               CDKparamValue (&params, 'N', TRUE),
                               CDKparamValue (&params, 'S', FALSE));
    if(home_scroll == 0){
        destroyCDKScreen(cdkscreen);
        endCDK();
        exit(EXIT_FAILURE);
    }
    if (CDKparamNumber(&params, 'c')){
        setCDKScrollItems (home_scroll, home_items, count, TRUE);
    }
    selection = activateCDKScroll (home_scroll, 0);

    char ch = '\0';
    while((ch = getch()) != 'q'){
        switch(ch){
            case 'n':
                break;
        }
    }

    //CDKfreeStrings (home_items);
    destroyCDKScroll(home_scroll);
    destroyCDKScreen(cdkscreen);
    endCDK();
    exit(EXIT_SUCCESS);
}

