#include <ncurses.h>
#include <string.h>
#include <math.h>

#include "menu.h"

/* @ret: 0 - entry was not drawn due to being conditional and condition being false
 *       1 - entry was drawn
 */
static int drawentry(struct menu *menu, int index)
{
    int winy, winx;
    int cury, curx;
    getmaxyx(menu->win, winy, winx);
    getyx(menu->win, cury, curx);

    union entryun *ent = menu->entries[index];

    if (ent->common.type == ENTRY_CONDITIONAL) {
        int res = ent->conditional.condition(menu);

        if (res)
            ent = ent->conditional.entry;
        else
            return 0;
    }

    if (index == menu->curentry)
        waddstr(menu->win, " *");
    else
        waddstr(menu->win, "  ");

    switch (ent->common.type) {
    case ENTRY_SELECTABLE: ;
        /* -2 for the borders */
        int newx = winx - strlen(ent->text.text) - 2;
        mvwaddstr(menu->win, cury, newx, ent->text.text);

        break;
    case ENTRY_ROULETTE:
        wprintw(menu->win, " %s: ", ent->roulette.text);
        waddstr(menu->win, "< ");

        int len = strlen(ent->roulette.alt[ent->roulette.curoption]);

        /* -1 for border, -2 for the "> " at the end */
        mvwaddstr(menu->win, cury, winx - 1 - 2 - len,
                ent->roulette.alt[ent->roulette.curoption]);
        waddstr(menu->win, "> ");
        break;
    case ENTRY_CONDITIONAL:
        return -1;
    }

    return 1;
}

static void drawmenu(struct menu *menu)
{
    int cury, curx;

    for (int i = 0; menu->entries[i]; i++) {
        getyx(menu->win, cury, curx);

        int ret = drawentry(menu, i);

        if (ret == 1)
            wmove(menu->win, cury + 1, 1);

        wrefresh(menu->win);
    }
}

static int getnextentry(struct menu *menu)
{

}

/* makes a centered menu with the corresponding entries */
int domenu(struct menu *menu)
{
    if (!menu || !menu->entries)
        return 1;

    int winy, winx;
    int cury, curx;
    getmaxyx(menu->win, winy, winx);

    wmove(menu->win, 1, 1);

    drawmenu(menu);
    wrefresh(menu->win);

    int ch;
    while ((ch = wgetch(menu->win))) {
        switch (ch) {
        case KEY_UP:
            if (menu->curentry != 0) {
                wmove(menu->win, menu->curentry + 1, 1);
                waddstr(menu->win, "  ");

                wmove(menu->win, menu->curentry, 1);
                waddstr(menu->win, " *");

                menu->curentry--;
            }

            break;
        case KEY_DOWN:
            if (menu->entries[menu->curentry + 1]) {
                wmove(menu->win, menu->curentry + 1, 1);
                waddstr(menu->win, "  ");

                wmove(menu->win, menu->curentry + 2, 1);
                waddstr(menu->win, " *");

                menu->curentry++;
            }

            break;
        case KEY_LEFT:
            if (menu->entries[menu->curentry]->common.type == ENTRY_ROULETTE) {
                struct roulent *tmp = &menu->entries[menu->curentry]->roulette;

                if (tmp->curoption != 0) {
                    tmp->curoption--;

                    int len = 1 + 2 + 1 + strlen(menu->entries[menu->curentry]->roulette.text) + 1 + 2;
                    getyx(menu->win, cury, curx);

                    wmove(menu->win, cury, len);
                    for (int i = len; i < winx - 3; i++)
                        waddch(menu->win, ' ');

                    len = winx - 3 - strlen(tmp->alt[tmp->curoption]);
                    mvwaddstr(menu->win, cury, len, tmp->alt[tmp->curoption]);
                }
            }

            break;
        case KEY_RIGHT:
            if (menu->entries[menu->curentry]->common.type == ENTRY_ROULETTE) {
                struct roulent *tmp = &menu->entries[menu->curentry]->roulette;

                if (tmp->alt[tmp->curoption + 1]) {
                    tmp->curoption++;

                    int len = 1 + 2 + 1 + strlen(menu->entries[menu->curentry]->text.text) + 1 + 2;
                    getyx(menu->win, cury, curx);

                    wmove(menu->win, cury, len);
                    for (int i = len; i < winx - 3; i++)
                        waddch(menu->win, ' ');

                    len = winx - 3 - strlen(tmp->alt[tmp->curoption]);
                    mvwaddstr(menu->win, cury, len, tmp->alt[tmp->curoption]);
                }
            }

            break;
        case '\n':
        case ' ':
            if (menu->entries[menu->curentry]->common.type == ENTRY_SELECTABLE)
                return menu->curentry;

            break;
        }
    }

    return -1;
}
