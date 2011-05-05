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

int move_next_page(WINDOW *win, status *page_start, int direction){
    if(!page_start)
        return -1;

    status *top = 0;
    status *bottom = 0;
    int y,x;

    getmaxyx(win,y,x);
    if(direction > 0){
        top = page_start;
    }
    else{
        status *p = page_start;
        int topy = y;
        while(p){
            topy -= p->y_max - p->y_min + 2;
            if(topy <= 0)
                break;
            p = p->prev;
        }
        if(!p)
            p = timelines[current_tl_index]->head;
        else
            p = p->next;
        top = p;
    }
    wclear(win);
    bottom = show_timeline(win,top,y,x);

    current_top_status[current_tl_index] = top;
    current_bottom_status[current_tl_index] = bottom;
    return 0;
}

void move_next(WINDOW *win, status *current, int direction){
    if(!current)
        return;

    status *next = 0;
    status *boundary = 0;
    if(direction > 0){
        next = current->next;
        boundary = current_bottom_status[current_tl_index];
    }
    else{
        next = current->prev;
        boundary = current_top_status[current_tl_index];
    }
    if(!next)
        return;

    if(current != boundary){
        wmove(win,current->y_min,0);
        show_status(win,current);
    }
    else
        move_next_page(win,current,direction);

    highlight_status(win,next);
    current_status[current_tl_index] = next;
}

void wait_command(WINDOW *win){
    char ch = '\0';
    while((ch = getch()) != 'q'){
        switch(ch){
            case 'n':
                // Compose new tweet
                break;
            case 'h':
                // move down one page
                move_next_page(win,current_bottom_status[current_tl_index],1);
                current_status[current_tl_index] = current_top_status[current_tl_index];
                highlight_status(win, current_status[current_tl_index]);
                break;
            case 'l':
                // move up one page
                move_next_page(win,current_top_status[current_tl_index],-1);
                current_status[current_tl_index] = current_top_status[current_tl_index];
                highlight_status(win, current_status[current_tl_index]);
                break;
            case 'j':
                // move down one tweet
                move_next(win, current_status[current_tl_index],1);
                break;
            case 'k':
                // move up one tweet
                move_next(win, current_status[current_tl_index],-1);
                break;
            case 'r':
                // refresh the current timeline
                break;
        }
    }
}

int main(){
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
    curs_set(0);

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
    curs_set(1);
    endwin(); 
    destroy_filters();
    for(int i = 0; i < TIMELINE_COUNT; ++i)
        destroy_timeline(timelines[i]);

    return 0;
}
