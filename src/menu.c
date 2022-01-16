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
    case ENTRY_INPUT:
        wprintw(menu->win, " %s: ", ent->input.text);
        mvwaddstr(menu->win, cury, winx - 3, "_");
        break;
    case ENTRY_CONDITIONAL:
        return -1;
    }

    return 1;

}

/* @ret: visual index of selected entry */
static int drawmenu(struct menu *menu)
{
    wclrtobot(menu->win);

    box(menu->win, 0, 0);
    wmove(menu->win, 1, 1);

    int cury, curx, tmp = -1;

    for (int i = 0; menu->entries[i]; i++) {
        getyx(menu->win, cury, curx);

        int ret = drawentry(menu, i);

        switch (ret) {
        case -1:
            return -1;
        case 1:
            if (i == menu->curentry)
                tmp = cury;

            wmove(menu->win, cury + 1, 1);
        }
    }

    return tmp;
}

/* get index of next visible entry in menu */
static int getnextentry(struct menu *menu)
{
    int inc = 1;

    while (1) {
       union entryun *ent = menu->entries[menu->curentry + inc];

       if (!ent) {
           return menu->curentry;
       } else if (ent->common.type == ENTRY_CONDITIONAL) {
           int res = ent->conditional.condition(menu);

           if (res)
               return menu->curentry + inc;
           else
               inc++;
       } else {
           return menu->curentry + inc;
       }
    }
}

/* get index of previous visible entry in menu */
static int getpreventry(struct menu *menu)
{
    if (menu->curentry == 0)
        return 0;

    int dec = 1;

    while (1) {
        union entryun *ent = menu->entries[menu->curentry - dec];

        if (menu->curentry - dec == 0) {
            return 0;
        } else if (ent->common.type == ENTRY_CONDITIONAL) {
            int res = ent->conditional.condition(menu);

            if (res)
                return menu->curentry - dec;
            else
                dec++;
        } else {
            return menu->curentry - dec;
        }
    }
}

/* get user input and then validate it
 * FIXME: Add bounds checking and prettify (grey bg) */
static void getinput(struct inent *input)
{
    int winx = 30, winy = 5;
    WINDOW *inputbox = newwin(winy, winx, 1, 1);

    keypad(inputbox, TRUE);
    curs_set(1);
    box(inputbox, 0, 0);

    int ch, cury, curx, len = 0, curind = 0;

    if (input->buf[0]) {
        len = strlen(input->buf);

        wmove(inputbox, 3, 1);

        waddstr(inputbox, input->buf);
    }

    wmove(inputbox, 3, 1);

    while ((ch = wgetch(inputbox))) {
        getyx(inputbox, cury, curx);

        switch (ch) {
        case KEY_LEFT:
            if (curind > 0) {
                curind--;
                wmove(inputbox, cury, curx - 1);
            }

            break;
        case KEY_RIGHT:
            if (curind < len) {
                curind++;
                wmove(inputbox, cury, curx + 1);
            }

            break;
        case '\n':
            if (input->validate) {
                char *err = input->validate(input->buf);

                if (err) {
                    mvwaddstr(inputbox, 1, 2, err);
                    wmove(inputbox, 3, 1);

                    for (int i = 1; i < winx - 1; i++)
                        waddch(inputbox, ' ');

                    input->buf[0] = 0;
                    curind = 0;
                    len = 0;

                    wmove(inputbox, 3, 1);
                    continue;
                }
            }

            delwin(inputbox);
            curs_set(0);
            return;
        case KEY_DC:
            if (curind >= 0 && curind < len) {
                memmove(input->buf + curind, input->buf + curind + 1, len - curind);

                len--;

                input->buf[len] = 0;
                mvwaddstr(inputbox, cury, 1, input->buf);

                waddch(inputbox, ' ');
                wmove(inputbox, cury, curx);
            }

            break;
        default:
            if (curx + 2 != winx && ch >= ' ' && ch <= '~') {
                memmove(input->buf + curind + 1, input->buf + curind, len - curind);
                input->buf[curind++] = ch;

                len++;

                input->buf[len + 1] = 0;
                mvwaddstr(inputbox, cury, 1, input->buf);

                wmove(inputbox, cury, curx + 1);
            }
        }
    }
}

/* makes a centered menu with the corresponding entries */
int domenu(struct menu *menu)
{
    if (!menu || !menu->entries)
        return 1;

    int winy, winx;
    getmaxyx(menu->win, winy, winx);
    box(menu->win, 0, 0);

    wmove(menu->win, 1, 1);

    drawmenu(menu);
    wrefresh(menu->win);

    int ch;
    while ((ch = wgetch(menu->win))) {
        switch (ch) {
        case KEY_UP: ;
            int prevent = getpreventry(menu);

            if (menu->curentry != prevent) {
                menu->curentry = prevent;

                wmove(menu->win, 1, 1);
                int viscury = drawmenu(menu);
                wmove(menu->win, viscury - 1, 1);
            }

            break;
        case KEY_DOWN: ;
            int nextent = getnextentry(menu);

            if (menu->curentry != nextent) {
                menu->curentry = nextent;

                wmove(menu->win, 1, 1);
                int viscury = drawmenu(menu);
                wmove(menu->win, viscury, 1);
            }

            break;
        case KEY_LEFT:
            if (menu->entries[menu->curentry]->common.type == ENTRY_ROULETTE) {
                struct roulent *tmp = &menu->entries[menu->curentry]->roulette;

                if (tmp->curoption != 0) {
                    tmp->curoption--;

                    wmove(menu->win, 1, 1);
                    int viscury = drawmenu(menu);
                    wmove(menu->win, viscury, 1);
                }
            }

            break;
        case KEY_RIGHT:
            if (menu->entries[menu->curentry]->common.type == ENTRY_ROULETTE) {
                struct roulent *tmp = &menu->entries[menu->curentry]->roulette;

                if (tmp->alt[tmp->curoption + 1]) {
                    tmp->curoption++;

                    wmove(menu->win, 1, 1);
                    int viscury = drawmenu(menu);
                    wmove(menu->win, viscury, 1);
                }
            }

            break;
        case '\n':
        case ' ': ;
            union entryun *ent = menu->entries[menu->curentry];

            if (ent->common.type == ENTRY_CONDITIONAL)
                ent = ent->conditional.entry;

            switch (ent->common.type) {
            case ENTRY_SELECTABLE:
                return menu->curentry;
            case ENTRY_CONDITIONAL:
                return -1;
            case ENTRY_INPUT:
                getinput(&ent->input);
                /* FIXME: Possibly inefficient */
                redrawwin(stdscr);
            }

            break;
        }
    }

    return -1;
}
