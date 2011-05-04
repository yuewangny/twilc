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

#define INPUT_MAX_SIZE 281
#define INPUT_WIN_HEIGHT 8
#define INPUT_WIN_WIDTH 60

#define PAD_LINES 1200

#define TWEET_STATE_DELETED 0
#define TWEET_TO_LOAD 0

int parse_timeline(char *filename, statuses *tl);

FILE *dbfp ;

WINDOW *create_newwin(int height, int width, int starty, int startx){
    WINDOW *local_win;

    local_win = newwin(height, width, starty, startx);
    box(local_win, 0 , 0);
    wrefresh(local_win);

    return local_win;
}

void backspace(WINDOW *win){
    int r,c;
    noecho();  
    nocbreak();  
    getyx(win, r, c);
    wmove(win, r, c-1);   
    delch(); 
    cbreak();  
    refresh();               
}

int *new_tweet(){
    int height = INPUT_WIN_HEIGHT;
    int width = INPUT_WIN_WIDTH;
    int starty = (LINES - height) / 2;
    int startx = (COLS - width) / 2;

    int *input = malloc(INPUT_MAX_SIZE*sizeof(int));
    memset(input,'\0',281);

    /* builds the new tweet window */
    refresh();
    WINDOW *new_win = create_newwin(height, width, starty, startx);
    wmove(new_win,1,1);
    keypad(new_win,TRUE);
    wrefresh(new_win);

    /* receives input */
    int ch = '\0';
    while((ch = wgetch(new_win))!=KEY_ENTER){
        switch(ch){
            case KEY_BACKSPACE:
                backspace(new_win);
                break;
            default:
                waddch(new_win,(char)ch);
                break;
        }
    }

    return input;
}

void destroy_win(WINDOW *local_win){ 
    box(local_win, ' ', ' '); 
    wborder(local_win, ' ', ' ', ' ',' ',' ',' ',' ',' ');
    wrefresh(local_win);
    delwin(local_win);
}

void filter_status_text(status *s){
    display_filter *current_filter;

    char **filtered_text = s->filtered_text;
    display_filter **filter_list = s->filter_list;

    int i = 0;
    char *begin = NULL;
    for(int k=0;k<FILTER_NUM;++k){
        char *temp = strstr(s->text,filters[k]->pattern);
        if(temp && (!begin || (begin && temp < begin))){
            begin = temp;
            current_filter = filters[k];
        }
    }

    char *end = s->text;
    char *prev = s->text;
    while(begin){
        prev = end;
        end = current_filter->get_pattern_end(begin);
        if(end == 0)
            break;

        int len = begin - prev;
        if(len > 0){
            filtered_text[i] = malloc((len + 1)*sizeof(char));
            strncpy(filtered_text[i],prev,len);
            filtered_text[i][len] = '\0';
            filter_list[i] = 0;
            printf("%s\n",filtered_text[i]);
            i++;
        }

        len = end - begin;
        filtered_text[i] = malloc((len + 1)*sizeof(char));
        strncpy(filtered_text[i],begin,len);
        filtered_text[i][len] = '\0';
        filter_list[i] = current_filter;
        printf("%s\n",filtered_text[i]);
        i++;

        begin = NULL;
        for(int k=0;k<FILTER_NUM;++k){
            char *temp = strstr(end,filters[k]->pattern);
            if(temp && (!begin || (begin && temp < begin))){
                begin = temp;
                current_filter = filters[k];
            }
        }
    }
    if(end)
        prev = end;
    if((*prev) != '\0'){
        filtered_text[i] = malloc((strlen(prev)+1)*sizeof(char));
        strcpy(filtered_text[i],prev);
        filter_list[i] = 0;
        printf("%s\n",filtered_text[i]);
        i ++;
    }
    s->filter_count = i;

}

void filter_display(WINDOW *win, status *p){
    int y,x;
    if(p->filter_count > 0){
        for(int j = 0; j < p->filter_count; j++){
            display_filter *fil = p->filter_list[j];
            if(fil)
                fil->before_filter(win);
            getyx(win,y,x);
            p->x_filter_begin[j] = x;
            p->y_filter_begin[j] = y;
            waddstr(win,p->filtered_text[j]);
            getyx(win,y,x);
            p->x_filter_end[j] = x;
            p->y_filter_end[j] = y;
            if(fil)
                fil->after_filter(win);
        }
    }
    else
        waddstr(win,p->text);
}

int show_status(WINDOW *win,status *p){
    int y,x;
    getyx(win,y,x);
    p->y_min = y;

    init_pair(5, COLOR_GREEN, COLOR_BLACK);
    wattron(win,COLOR_PAIR(5));
    waddstr(win,p->composer.screen_name);
    wattroff(win,COLOR_PAIR(5));
    waddch(win,'\n');

    filter_display(win,p);

    getyx(win,y,x);
    p->y_max = y;

    waddch(win,'\n');
    fprintf(dbfp,"%s\n",p->text);
}

/*
 * Merge new tweets with current timeline.
 */
int load_timeline(char *tmpfile, statuses *tl){
    LIBXML_TEST_VERSION
    statuses *toptweets = malloc(sizeof(statuses));
    toptweets->count = 0;
    parse_timeline(tmpfile,toptweets);

    if(tl->count == 0){ // No old tweets
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
            if(result > 0){ // new tweet
                prev = top;
                top = top->next;
            }
            else if(result < 0){ // old tweet already deleted
                oldtop->id = TWEET_STATE_DELETED;
                oldtop = oldtop->next;
            }
            else{
                prev->next = oldtop;
                oldtop->prev = prev;
                break;
            }
        }

        // Gap between new and old tweets
        if(!top){
            status *temp = malloc(sizeof(status));
            temp->id = TWEET_TO_LOAD;
            temp->text = "\nGap\n";
            prev->next = temp;
            temp->prev = prev;
            temp->next = oldtop;
            oldtop->prev = temp;
        }

        // free deleted tweets
        status *p = tl->head;
        while(p && p != oldtop){
            prev = p;
            p = p->next;
            free(prev);
        }

        tl->head = toptweets->head;
    }
}

status *show_timeline(WINDOW *win, status *p,int height, int width){ 
    char border[width+1];
    for(int i=0;i<width;i++)
        border[i] = '-';
    border[width] = '\0';

    wmove(win,0,0);
    while(p){
        show_status(win,p);
        waddstr(win,border);
        if(p->y_max >= height-3)
            break;
        p = p->next;
    }
    return p;
}

void highlight_status(WINDOW *win, status *s){
    for(int line = s->y_min; line <= s->y_max; ++line){
        wmove(win,line,0);
        wchgat(win,-1,A_REVERSE,0,NULL);
    }
    fprintf(dbfp,"%d %d\n",s->y_min,s->y_max);
    wrefresh(win);
}

void wait_command(WINDOW *win){
    char ch = '\0';
    status *current;
    while((ch = getch()) != 'q'){
        switch(ch){
            case 'n':
                new_tweet();
                break;
            case 'j':
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
                break;
        }
    }
}

int main(){
    dbfp= fopen("debug.info","w");

    // Reading User information
    clit_config *config = malloc(sizeof(clit_config));
    if(parse_config(config) == -1){
        char *access_token;
        char *access_token_secret;
        char *user_id;
        char *screen_name;

        if(oauth_authorize(&access_token,&access_token_secret,&user_id,&screen_name)){
            printf("Authorization failed\n");
            free(config);
            exit(0);
        }
        else{
            printf("access_token:%s\naccess_token_secret:%s\nscreen_name:%s\nuser_id:%s\n",access_token,access_token_secret,screen_name,user_id);
            init_config(access_token,access_token_secret,user_id,screen_name,config);
            save_config(config);
            free(config);
            exit(0);
        }
    }
    //printf("key:%s,secret:%s\n",config->key,config->secret);
    init_oauth(config->key,config->secret);

    // Create display filters for status texts
    create_filters();

    // Initialize timelines
    for(int i = 0; i < TIMELINE_COUNT; ++i){
        timelines[i] = malloc(sizeof(statuses));
        timelines[i]->count = 0;
    }

    statuses *home = timelines[0];
    char *tmpfile = get_home();
    load_timeline(tmpfile,home);
    for(status *s = home->head; s; s = s->next){
        printf("%s\n",s->text);
        filter_status_text(s);
    }
    current_status[0] = timelines[0]->head;
    current_top_status[0] = timelines[0]->head;

    current_tl_index = 0;

    setlocale(LC_ALL,"");
    initscr();
    if(has_colors() == FALSE)
    {   endwin();
        printf("Your terminal does not support color\n");
        exit(1);
    }
    raw();
    keypad(stdscr,TRUE);
    noecho();
    start_color();

    move(0,0);
    addstr("Loading......");
    refresh();

    int height,width;
    getmaxyx(stdscr,height,width);
    WINDOW *tlwin = newwin(height,width,0,0);

    move(0,0);
    current_bottom_status[current_tl_index] = show_timeline(tlwin,current_status[current_tl_index],height,width);
    wrefresh(tlwin);
    highlight_status(tlwin,current_status[current_tl_index]);

    wait_command(tlwin);

    for(status *s = home->head; s; s = s->next){
        if(s->id)
            free(s->id);
        for(int i=0;i<s->filter_count;++i){
            if(s->filtered_text[i])
                free(s->filtered_text[i]);
        }
        free(s);
    }
    delwin(tlwin);
    endwin(); 
    fclose(dbfp);

    return 0;
}
