#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <libxml/parser.h>
#include <locale.h>

#include "twitter.h"
#include "config.h"
#include "twiauth.h"
#include "twiaction.h"
#include "filter.h"
#include "twiparse.h"
#include "ui.h"


void wait_command(WINDOW *win){
    char ch = '\0';
    status *current;
    while((ch = getch()) != 'q'){
        switch(ch){
            case 'n':
                // Compose new tweet
                break;
            case 'j':
                // move down one tweet
                current = current_status[current_tl_index];
                if(current && current->next 
                        && current != current_bottom_status[current_tl_index]
                        ){
                    wmove(win,current->y_min,0);
                    show_status(win,current);
                    current_status[current_tl_index] = current->next;
                    highlight_status(win,current_status[current_tl_index]);
                }
                break;
            case 'k':
                // move up one tweet
                break;
        }
    }
}

int main(){
    dbfp= fopen("debug.info","w");

    clit_config *config = malloc(sizeof(clit_config));
    if(parse_config(config) == -1){
        // first run
        int res = authorize(config);
        if(res == -1){
            printf("Please retry.\n");
        }
        else
            printf("Authorization finished. Run the program again.\n");
        free(config);
        exit(0);
    }

    init_oauth(config->key,config->secret);
    create_filters();
    init_timelines();

    setlocale(LC_ALL,"");
    initscr();

    int height,width;
    getmaxyx(stdscr,height,width);
    WINDOW *tlwin = newwin(height,width,0,0);

    if(has_colors() == FALSE)
        goto exit_clit;
    raw();
    keypad(stdscr,TRUE);
    noecho();
    start_color();

    move(0,0);
    refresh();
    current_bottom_status[current_tl_index] = show_timeline(tlwin,current_status[current_tl_index],height,width);
    wrefresh(tlwin);
    highlight_status(tlwin,current_status[current_tl_index]);

    wait_command(tlwin);


exit_clit:
    if(tlwin)
        delwin(tlwin);
    endwin(); 
    destroy_filters();
    for(int i = 0; i < TIMELINE_COUNT; ++i)
        destroy_timeline(timelines[i]);
    if(dbfp)
        fclose(dbfp);

    return 0;
}
