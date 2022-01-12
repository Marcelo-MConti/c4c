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

    menu.entries = (struct entry *[]) {
        (struct entry *) (struct textent []) {{
            ENTRY_SELECTABLE, "START"
        }},
        
        (struct entry *) (struct textent []) {{
            ENTRY_SELECTABLE, "QUIT"
        }},

        (struct entry *) (struct roulent []) {{
            ENTRY_ROULETTE, "PLAYERS",
            (char *[]) { "PL. VS. PL.", "PLAYER VS. PC", "PC VS. PC" },
        }},

        0
    };
   
    int entry = domenu(stdscr, &menu, 30, 6);

    if (entry == 0) {
    }
    addstr("ok");
    refresh();
    
    getch();

    endwin();
}
