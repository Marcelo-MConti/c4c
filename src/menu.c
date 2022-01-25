#include <ncurses.h>
#include <string.h>

#include <err.h>

#include "util.h"
#include "menu.h"

enum {
    PREVIOUS,
    NEXT
};

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
        if (ent->conditional.condition(menu))
            ent = ent->conditional.entry;
        else
            return 0;
    }

    if (index == menu->cur_entry)
        mvwaddch(menu->win, cury, 2, '*' | A_BOLD);
    else
        mvwaddch(menu->win, cury, 2, ' ');

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

        if (!ent->input.buf[0]) {
            mvwaddch(menu->win, cury, winx - 3, '_');
        } else {
            int len = strlen(ent->input.buf);

            wmove(menu->win, cury, winx - ((len < 15) ? len : 15) - 2);
            print_truncate(menu->win, ent->input.buf, len, 15);
        }

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

        switch (draw_entry(menu, i)) {
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

/* @ret: index of previous/next visible entry in menu */
static int get_entry_whence(struct menu *menu, int whence)
{
    int inc = menu->cur_entry;

    while (1) {
        union entry_un *ent;

        if (whence == PREVIOUS && inc == 0)
            return 0;

        if (whence == PREVIOUS)
            ent = menu->entries[--inc];
        else
            ent = menu->entries[++inc];

        if (whence == NEXT && !ent)
            return menu->cur_entry;

        if (ent->common.type == ENTRY_CONDITIONAL) {
            if (ent->conditional.condition(menu))
                return inc;
        } else {
            return inc;
        }
    }
}

/* get user input and then validate it
 * FIXME: Add bounds checking */
static void get_input(struct in_ent *input)
{
    init_pair(1, COLOR_RED, COLOR_BLACK);

    int COLS, LINES;
    getmaxyx(stdscr, LINES, COLS);

    int winx = 30, winy = 5;

    int offx = (COLS - winx) / 2;
    int offy = 3;

    WINDOW *input_box = newwin(winy, winx, offy, offx);

    keypad(input_box, TRUE);
    curs_set(1);
    box(input_box, 0, 0);

    int ch, cury, curx, len = 0, cur_ind = 0;

    wmove(input_box, 3, 1);
    fill(input_box, 1, winx - 2, '#' | A_BOLD);

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
            if (input->validate && input->buf[0] != 0) {
                char *err = input->validate(input->buf);

                if (err) {
                    fill(input_box, 1, winx - 2, ' ');

                    wattrset(input_box, COLOR_PAIR(1));
                    mvwaddstr(input_box, 1, (winx - strlen(err)) / 2, err);
                    wattrset(input_box, COLOR_PAIR(0));

                    wmove(input_box, 3, 1);
                    fill(input_box, 1, winx - 2, '#' | A_BOLD);

                    input->buf[0] = 0;
                    cur_ind = 0;
                    len = 0;

                    continue;
                }
            }

            reset_color_pairs();
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

    keypad(menu->win, TRUE);
    box(menu->win, 0, 0);

    wmove(menu->win, 1, 1);

    draw_menu(menu);

    int ch;
    while ((ch = wgetch(menu->win))) {
        switch (ch) {
        case KEY_UP: ;
            int prev_ent = get_entry_whence(menu, PREVIOUS);

            if (menu->cur_entry != prev_ent) {
                menu->cur_entry = prev_ent;

                wmove(menu->win, 1, 1);
                int vis_cur_y = draw_menu(menu);
                wmove(menu->win, vis_cur_y - 1, 1);
            }

            break;
        case KEY_DOWN: ;
            int next_ent = get_entry_whence(menu, NEXT);

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

                redrawwin(stdscr);
                wrefresh(stdscr);

                draw_menu(menu);

                for (int i = 0; menu->center[i].win; i++) {
                    redrawwin(menu->center[i].win);
                    wnoutrefresh(menu->center[i].win);
                }

                doupdate();
            }

            break;
        case KEY_RESIZE:
            CHECK_TERMSIZE();

            center_wins(menu->center);
        }
    }

    return -1;
}
