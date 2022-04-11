#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <ncurses.h>

#include "util.h"
#include "logo.h"
#include "game.h"
#include "menu.h"

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
    /* Required for ncurses to support UTF-8 */
    setlocale(LC_CTYPE, "");
    initscr();

#ifdef C4C_COLOR
    if (!has_colors())
        errx(1, "This terminal does not support colors, which is required for c4c to run.");
    start_color();
#endif

    CHECK_TERMSIZE();

    cbreak();
    noecho();
    curs_set(0);

    int max_logo_width = 0;

    for (int i = 0; i < ARR_SIZE(logo); i++) {
        int len = utf8len(logo[i]);

        if (len > max_logo_width)
            max_logo_width = len;
    }

    struct menu menu = {0};

    menu.center = calloc(3, sizeof(struct win_off));
    menu.center[0].off_offy = -4;
    menu.center[1].off_offy = 4;

    int offx = (COLS - max_logo_width) / 2;
    int offy = (LINES - ARR_SIZE(logo)) / 2 + menu.center[0].off_offy;
    WINDOW *logo_win = newwin(ARR_SIZE(logo), max_logo_width, offy, offx);

    menu.center[0].win = logo_win;

    for (int i = 0; i < ARR_SIZE(logo); i++) {
        waddstr(logo_win, logo[i]);
        wmove(logo_win, i + 1, 0);
    }

    wrefresh(logo_win);

    offx = (COLS - 30) / 2;
    offy = (LINES - 6) / 2 + menu.center[1].off_offy;
    WINDOW *menu_win = newwin(6, 30, offy, offx);

    menu.center[1].win = menu_win;
    menu.win = menu_win;

    menu.center[2].win = NULL;

    menu.entries = (union entry_un *[]) {
        (union entry_un *) (struct text_ent []) {{
            ENTRY_SELECTABLE, "START"
        }},

        (union entry_un *) (struct text_ent []) {{
            ENTRY_SELECTABLE, "QUIT"
        }},

        (union entry_un *) (struct roul_ent []) {{
            ENTRY_ROULETTE, 0, "PMODE",
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

    while (1) {
        int entry = do_menu(&menu);

        switch (entry) {
        case 0: ;
            start_game(7, 6, menu.entries[2]->roulette.cur_option);

            break;
        case 1:
            nocbreak();
            echo();
            curs_set(1);
            endwin();

            free(menu.center);

            return 0;
       }

       redrawwin(stdscr);
       wrefresh(stdscr);

       redrawwin(logo_win);
       wrefresh(logo_win);
    }
}
