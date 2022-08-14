#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <curses.h>

#include "util.h"
#include "logo.h"
#include "game.h"
#include "menu.h"
#include "center.h"

char *validate_host(char *buf)
{
    if (strchr(buf, ':'))
        return NULL;
    else
        return "Host format: host:port";
}

int check_netplay(struct menu *menu)
{
    struct roul_ent *entry = &(*menu->entries)[2]->roulette;
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

    center_init();

    int max_logo_width = 0;
    for (size_t i = 0; i < ARR_SIZE(logo); i++) {
        int len = utf8len(logo[i]);
        if (len > max_logo_width)
            max_logo_width = len;
    }

    struct menu menu = {0};
    union entry_un *entries[] = (union entry_un *[]) {
        (union entry_un *) (struct text_ent []) {{
            ENTRY_SELECTABLE, "START"
        }},

        (union entry_un *) (struct text_ent []) {{
            ENTRY_SELECTABLE, "QUIT"
        }},

        (union entry_un *) (struct roul_ent []) {{
            ENTRY_ROULETTE, 0, "PLAY MODE",
            (char *[]) { "PL VS. PL", "PL VS. PC", "PC VS. PC", "NETPLAY", 0 }
        }},

        (union entry_un *) (struct cond_ent []) {{
            ENTRY_CONDITIONAL,
            (union entry_un *) (struct in_ent []) {{
                ENTRY_INPUT, 255, "HOST", (char [256]) {0}, validate_host
            }},
            check_netplay
        }},

        (union entry_un *) (struct cond_ent []) {{
            ENTRY_CONDITIONAL,
            (union entry_un *) (struct roul_ent []) {{
                ENTRY_ROULETTE, 0, "P2P MODE",
                (char *[]) { "SERVER", "CLIENT", 0 }
            }},
            check_netplay
        }},

        0
    };
    menu.entries = &entries;

    int winy = ARR_SIZE(logo) + ARR_SIZE(entries) - 1 + 6, winx = MAX(max_logo_width, 30);
    WINDOW *main_win = newwin(winy, winx, 30, 30);

    WINDOW *logo_win = derwin(main_win, ARR_SIZE(logo), winx, 0, 0);

    int logox = (winx - max_logo_width) / 2;
    for (int i = 0; i < ARR_SIZE(logo); i++) {
        wmove(logo_win, i, logox);
        waddstr(logo_win, logo[i]);
    }

    WINDOW *menu_win = derwin(main_win, 7, winx, ARR_SIZE(logo) + 1, 0);
    menu.win = menu_win;

    center_win_add(main_win, 0, 0, 0);
    center_win_trigger();
    refresh();

    while (1) {
        int entry = do_menu(&menu);

        switch (entry) {
        case 0: ;
            start_game(7, 6, (*menu.entries)[2]->roulette.cur_option);

            break;
        case 1:
            nocbreak();
            echo();
            curs_set(1);
            endwin();

            return 0;
       }

       redrawwin(main_win);
       wrefresh(main_win);
    }
}
