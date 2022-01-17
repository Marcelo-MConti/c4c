#include <stdlib.h>
#include <string.h>
#include <ncurses.h>

#include <err.h>

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

char *validate_host(char *buf)
{
    if (strchr(buf, ':'))
        return NULL;
    else
        return "Host format: host:port";
}

int check_netplay(struct menu *menu)
{
    struct roul_ent *entry = &menu->entries[2]->roulette;
    return entry->cur_option == PLAY_NET;
}

int main()
{
    initscr();

    if (!has_colors())
        errx(1, "This terminal does not support colors, which is required for c4c to run.");

    int termx, termy;
    getmaxyx(stdscr, termy, termx);

    if (termy < 10 || termx < 40)
        errx(1, "Needs at least a 40x10 to terminal to work.");

    start_color();
    cbreak();
    noecho();
    curs_set(0);

    struct menu menu = {0};
    menu.win = stdscr;

    menu.entries = (union entry_un *[]) {
        (union entry_un *) (struct text_ent []) {{
            ENTRY_SELECTABLE, "START"
        }},

        (union entry_un *) (struct text_ent []) {{
            ENTRY_SELECTABLE, "QUIT"
        }},

        (union entry_un *) (struct roul_ent []) {{
            ENTRY_ROULETTE, 0, "PLAYERS",
            (char *[]) { "PL. VS. PL.", "PLAYER VS. PC", "PC VS. PC", "NETPLAY", 0 },
        }},

        (union entry_un *) (struct cond_ent []) {{
            ENTRY_CONDITIONAL,
            (union entry_un *) (struct in_ent []) {{
                ENTRY_INPUT, 255, "HOST", (char [256]) {0}, validate_host
            }},
            check_netplay
        }},

        0
    };

    keypad(stdscr, TRUE);

    int entry = do_menu(&menu);

    switch (entry) {
    case 0:
        quit();
    case 1: ;
        struct game_info gi;
        gi.play_mode = menu.entries[2]->roulette.cur_option;
        gi.width = 7;
        gi.height = 6;

        start_game(&gi);
    }

    getch();

    endwin();
}
