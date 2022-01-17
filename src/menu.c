#include <ncurses.h>
#include <string.h>

#include "menu.h"

/* @ret: 0 - entry was not drawn due to being conditional and condition being false
 *       1 - entry was drawn
 */
static int draw_entry(struct menu *menu, int index)
{
    int winy, winx;
    int cury, curx;
    getmaxyx(menu->win, winy, winx);
    getyx(menu->win, cury, curx);

    union entry_un *ent = menu->entries[index];

    if (ent->common.type == ENTRY_CONDITIONAL) {
        int res = ent->conditional.condition(menu);

        if (res)
            ent = ent->conditional.entry;
        else
            return 0;
    }

    if (index == menu->cur_entry)
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

        int len = strlen(ent->roulette.alt[ent->roulette.cur_option]);

        /* -1 for border, -2 for the "> " at the end */
        mvwaddstr(menu->win, cury, winx - 1 - 2 - len,
                ent->roulette.alt[ent->roulette.cur_option]);
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
static int draw_menu(struct menu *menu)
{
    wclrtobot(menu->win);

    box(menu->win, 0, 0);
    wmove(menu->win, 1, 1);

    int cury, curx, tmp = -1;

    for (int i = 0; menu->entries[i]; i++) {
        getyx(menu->win, cury, curx);

        int ret = draw_entry(menu, i);

        switch (ret) {
        case -1:
            return -1;
        case 1:
            if (i == menu->cur_entry)
                tmp = cury;

            wmove(menu->win, cury + 1, 1);
        }
    }

    return tmp;
}

/* get index of next visible entry in menu */
static int get_next_entry(struct menu *menu)
{
    int inc = 1;

    while (1) {
       union entry_un *ent = menu->entries[menu->cur_entry + inc];

       if (!ent) {
           return menu->cur_entry;
       } else if (ent->common.type == ENTRY_CONDITIONAL) {
           int res = ent->conditional.condition(menu);

           if (res)
               return menu->cur_entry + inc;
           else
               inc++;
       } else {
           return menu->cur_entry + inc;
       }
    }
}

/* get index of previous visible entry in menu */
static int get_prev_entry(struct menu *menu)
{
    if (menu->cur_entry == 0)
        return 0;

    int dec = 1;

    while (1) {
        union entry_un *ent = menu->entries[menu->cur_entry - dec];

        if (menu->cur_entry - dec == 0) {
            return 0;
        } else if (ent->common.type == ENTRY_CONDITIONAL) {
            int res = ent->conditional.condition(menu);

            if (res)
                return menu->cur_entry - dec;
            else
                dec++;
        } else {
            return menu->cur_entry - dec;
        }
    }
}

/* get user input and then validate it
 * FIXME: Add bounds checking and prettify (grey bg) */
static void get_input(struct in_ent *input)
{
    int winx = 30, winy = 5;
    WINDOW *input_box = newwin(winy, winx, 1, 1);

    keypad(input_box, TRUE);
    curs_set(1);
    box(input_box, 0, 0);

    int ch, cury, curx, len = 0, cur_ind = 0;

    wmove(input_box, 3, 1);
    for (int i = 1; i < winx - 1; i++)
        waddch(input_box, '#' | A_BOLD);
    wmove(input_box, 3, 1);

    if (input->buf[0]) {
        len = strlen(input->buf);
        waddstr(input_box, input->buf);
    }

    wmove(input_box, 3, 1);

    while ((ch = wgetch(input_box))) {
        getyx(input_box, cury, curx);

        switch (ch) {
        case KEY_LEFT:
            if (cur_ind > 0) {
                cur_ind--;
                wmove(input_box, cury, curx - 1);
            }

            break;
        case KEY_RIGHT:
            if (cur_ind < len) {
                cur_ind++;
                wmove(input_box, cury, curx + 1);
            }

            break;
        case '\n':
            if (input->validate) {
                char *err = input->validate(input->buf);

                if (err) {
                    mvwaddstr(input_box, 1, 2, err);
                    wmove(input_box, 3, 1);

                    for (int i = 1; i < winx - 1; i++)
                        waddch(input_box, '#' | A_BOLD);

                    input->buf[0] = 0;
                    cur_ind = 0;
                    len = 0;

                    wmove(input_box, 3, 1);
                    continue;
                }
            }

            delwin(input_box);
            curs_set(0);
            return;
        case KEY_DC:
            if (cur_ind >= 0 && cur_ind < len) {
                memmove(input->buf + cur_ind, input->buf + cur_ind + 1, len - cur_ind);

                len--;

                input->buf[len] = 0;
                mvwaddstr(input_box, cury, 1, input->buf);

                waddch(input_box, '#' | A_BOLD);
                wmove(input_box, cury, curx);
            }

            break;
        case KEY_HOME:
            cur_ind = 0;
            wmove(input_box, cury, 1);

            break;
        case KEY_END:
            cur_ind = len;
            wmove(input_box, cury, len + 1);

            break;
        default:
            if (curx + 2 != winx && ch >= ' ' && ch <= '~') {
                memmove(input->buf + cur_ind + 1, input->buf + cur_ind, len - cur_ind);
                input->buf[cur_ind++] = ch;

                len++;

                input->buf[len] = 0;
                mvwaddstr(input_box, cury, 1, input->buf);

                wmove(input_box, cury, curx + 1);
            }
        }
    }
}

/* makes a centered menu with the corresponding entries */
int do_menu(struct menu *menu)
{
    if (!menu || !menu->entries)
        return 1;

    int winy, winx;
    getmaxyx(menu->win, winy, winx);
    box(menu->win, 0, 0);

    wmove(menu->win, 1, 1);

    draw_menu(menu);
    wrefresh(menu->win);

    int ch;
    while ((ch = wgetch(menu->win))) {
        switch (ch) {
        case KEY_UP: ;
            int prev_ent = get_prev_entry(menu);

            if (menu->cur_entry != prev_ent) {
                menu->cur_entry = prev_ent;

                wmove(menu->win, 1, 1);
                int vis_cur_y = draw_menu(menu);
                wmove(menu->win, vis_cur_y - 1, 1);
            }

            break;
        case KEY_DOWN: ;
            int next_ent = get_next_entry(menu);

            if (menu->cur_entry != next_ent) {
                menu->cur_entry = next_ent;

                wmove(menu->win, 1, 1);
                int vis_cur_y = draw_menu(menu);
                wmove(menu->win, vis_cur_y, 1);
            }

            break;
        case KEY_LEFT:
            if (menu->entries[menu->cur_entry]->common.type == ENTRY_ROULETTE) {
                struct roul_ent *tmp = &menu->entries[menu->cur_entry]->roulette;

                if (tmp->cur_option != 0) {
                    tmp->cur_option--;

                    wmove(menu->win, 1, 1);
                    int vis_cur_y = draw_menu(menu);
                    wmove(menu->win, vis_cur_y, 1);
                }
            }

            break;
        case KEY_RIGHT:
            if (menu->entries[menu->cur_entry]->common.type == ENTRY_ROULETTE) {
                struct roul_ent *tmp = &menu->entries[menu->cur_entry]->roulette;

                if (tmp->alt[tmp->cur_option + 1]) {
                    tmp->cur_option++;

                    wmove(menu->win, 1, 1);
                    int vis_cur_y = draw_menu(menu);
                    wmove(menu->win, vis_cur_y, 1);
                }
            }

            break;
        case '\n':
        case ' ': ;
            union entry_un *ent = menu->entries[menu->cur_entry];

            if (ent->common.type == ENTRY_CONDITIONAL)
                ent = ent->conditional.entry;

            switch (ent->common.type) {
            case ENTRY_SELECTABLE:
                return menu->cur_entry;
            case ENTRY_CONDITIONAL:
                return -1;
            case ENTRY_INPUT:
                get_input(&ent->input);
                /* FIXME: Possibly inefficient */
                redrawwin(stdscr);
            }

            break;
        }
    }

    return -1;
}
