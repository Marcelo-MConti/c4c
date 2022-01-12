#include <ncurses.h>
#include <string.h>
#include <math.h>

#include "menu.h"

/* makes a centered menu with the corresponding entries */
int domenu(WINDOW *win, struct menu *menu, int x, int y) 
{
    if (!menu || !menu->entries || menu->entrycnt == 0)
        return 1;

    int winx, winy;
    int cury, curx;
    getmaxyx(stdscr, winy, winx);

    int offx = round((double)(winx - x) / 2);
    int offy = round((double)(winy - y) / 2);

    WINDOW *menuwin = newwin(y, x, offy, offx);
    keypad(menuwin, 1);

    box(menuwin, 0, 0);

    wmove(menuwin, 1, 1);

    int i;
    for (i = 0; i < menu->entrycnt; i++) {
        if (i == menu->curentry)
            waddstr(menuwin, " >");
        else
            waddstr(menuwin, "  ");

        getyx(menuwin, cury, curx);

        switch (menu->entries[i].type) {
        case ENTRY_SELECTABLE: ;
            /* -2 for the borders */
            int newx = x - strlen(menu->entries[i].text) - 2;
            mvwaddstr(menuwin, cury, newx, menu->entries[i].text);

            break;
        case ENTRY_ROULETTE:
            wprintw(menuwin, " %s: ", menu->entries[i].text);
            waddstr(menuwin, "< ");

            struct rlent *data = (struct rlent *)menu->entries[i].data;  
            int len = strlen(data->alt[data->curoption]);

            /* -1 for border, -2 for the "> " at the end */ 
            mvwaddstr(menuwin, cury, x - 1 - 2 - len, data->alt[data->curoption]);
            waddstr(menuwin, "> ");
            break;
        }

        wmove(menuwin, cury + 1, 1);
    }

    wrefresh(menuwin);
  
    int ch;
    while ((ch = wgetch(menuwin))) {
        switch (ch) {
        case KEY_UP: 
            if (menu->curentry != 0) {
                wmove(menuwin, menu->curentry + 1, 1);
                waddstr(menuwin, "  ");
                
                wmove(menuwin, menu->curentry, 1);
                waddstr(menuwin, " >");
                
                menu->curentry--;
            }

            break;
        case KEY_DOWN:
            if (menu->curentry + 1 != menu->entrycnt) {
                wmove(menuwin, menu->curentry + 1, 1);
                waddstr(menuwin, "  ");
                
                wmove(menuwin, menu->curentry + 2, 1);
                waddstr(menuwin, " >");
                
                menu->curentry++;
            }

            break;
        case KEY_LEFT:
            if (menu->entries[menu->curentry].type == ENTRY_ROULETTE) {
                struct rlent *data = (struct rlent *)menu->entries[menu->curentry].data;

                if (data->curoption != 0) {
                    data->curoption--;
                    
                    int len = 1 + 2 + 1 + strlen(menu->entries[menu->curentry].text) + 1 + 2;
                    getyx(menuwin, cury, curx);

                    wmove(menuwin, cury, len);
                    for (int i = len; i < x - 3; i++)
                        waddch(menuwin, ' ');

                    len = x - 3 - strlen(data->alt[data->curoption]);
                    mvwaddstr(menuwin, cury, len, data->alt[data->curoption]);

                    *((short *)data->store) = data->curoption;
                }
            }

            break;
        case KEY_RIGHT:
            if (menu->entries[menu->curentry].type == ENTRY_ROULETTE) {
                struct rlent *data = (struct rlent *)menu->entries[menu->curentry].data;

                if (data->curoption + 1 != data->entrycnt) {
                    data->curoption++;

                    int len = 1 + 2 + 1 + strlen(menu->entries[menu->curentry].text) + 1 + 2;
                    getyx(menuwin, cury, curx);

                    wmove(menuwin, cury, len);
                    for (int i = len; i < x - 3; i++)
                        waddch(menuwin, ' ');

                    len = x - 3 - strlen(data->alt[data->curoption]);
                    mvwaddstr(menuwin, cury, len, data->alt[data->curoption]);
                    
                    *((short *)data->store) = data->curoption;
                }
            }

            break;
        case '\n': 
        case ' ':
            if (menu->entries[menu->curentry].type == ENTRY_SELECTABLE) {
                struct selent *data = menu->entries[menu->curentry].data;
                
                if (data->attr & SELENT_MENU_RETURN) {
                    delwin(menuwin);
                    return menu->curentry;
                } else {
                    data->func(data->arg);
                }
            }

            break;
        }
    }

    return 1;
}
