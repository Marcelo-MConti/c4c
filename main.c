#include <stdlib.h>
#include <ncurses.h>

#include "logo.h"
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

int validate(char *buf)
{

}

int checknetplay(struct menu *menu)
{
    struct roulent *entry = (struct roulent *)menu->entries[2];

    return entry->curoption == PLAY_NET;
}

int main()
{
    initscr();
    cbreak(); 
    noecho();
    curs_set(0);

    struct menu menu;

    menu.entries = (union entryun *[]) {
        (union entryun *) (struct textent []) {{
            ENTRY_SELECTABLE, "START"
        }},
        
        (union entryun *) (struct textent []) {{
            ENTRY_SELECTABLE, "QUIT"
        }},

        (union entryun *) (struct roulent []) {{
            ENTRY_ROULETTE, "PLAYERS",
            (char *[]) { "PL. VS. PL.", "PLAYER VS. PC", "PC VS. PC", "NETPLAY" },
        }},

        (union entryun *) (struct condent []) {{
            ENTRY_CONDITIONAL, ENTRY_INPUT,
            (union entryun *) (struct inent []) {{
                ENTRY_INPUT, "HOST", (char [256]) {0}, validate
            }},
            checknetplay
        }},

        0
    };

//    int entry = domenu(stdscr, &menu);

//    if (entry == 0) {
//    }
    addstr("ok");
    refresh();
    
    getch();

    endwin();
}
