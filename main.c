#include <stdlib.h>
#include <ncurses.h>

#include "game.h"
#include "menu.h"

void quit()
{
    nocbreak();
    echo();
    curs_set(1);
    endwin();
    
    exit(0);
}

int main()
{
    initscr();
    cbreak(); 
    noecho();
    curs_set(0);

    struct menu menu;
    
    menu.curentry = 0;
    menu.entrycnt = 3;
    menu.entries = (struct entry []) {
        {
            ENTRY_SELECTABLE, "START", 
                &(struct selent) {
                    .func = (void (*)(void *))startgame,
                    .arg = &(struct gameinfo) {0},
                    .attr = SELENT_MENU_RETURN
                } 
        },
        
        {
            ENTRY_SELECTABLE, "QUIT", 
                &(struct selent) { 
                    .func = quit 
                }
        },

        {
            ENTRY_ROULETTE, "PLAYERS",
                &(struct rlent) { 
                    .alt = (char *[]) { "PL. VS. PL.", "PLAYER VS. PC", "PC VS. PC" },
                    .entrycnt = 3
                }
        }
    };

    ((struct rlent *)menu.entries[2].data)->store = 
        &((struct gameinfo *)((struct selent *)menu.entries[0].data)->arg)->playmode;
    
    int entry = domenu(stdscr, &menu, 30, 6);

    if (entry == 0) {
        struct selent *data = (struct selent *)menu.entries[0].data;
        data->func(data->arg);
    }

    addstr("ok");
    refresh();
    
    getch();

    endwin();
}
