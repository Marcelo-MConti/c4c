#include <ncurses.h>
#include <string.h>

#include <err.h>

#include "util.h"
#include "menu.h"

enum {
    PREVIOUS,
    NEXT
};

static bool draw_entry(struct menu *menu, int index)
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
            return false;
    }

    mvwaddch(menu->win, cury, 2,
            index == menu->cur_entry ? '*' | A_BOLD : ' ');

    switch (ent->common.type) {
    case ENTRY_TEXT:
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

    return true;
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

        if (draw_entry(menu, i)) {
            if (i == menu->cur_entry)
                tmp = cury;
            wmove(menu->win, cury + 1, 1);
        } else {
            return -1;
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

        ent = menu->entries[whence == PREVIOUS ? --inc : ++inc];

        if (whence == NEXT && !ent)
            return menu->cur_entry;

        if (ent->common.type == ENTRY_CONDITIONAL) {
            if (ent->conditional.condition(menu))
                return inc;
            // else continue;
        } else {
            return inc;
        }
    }
}

/* draw the input bar */
static void draw_input(WINDOW *win, int x, int y, int width,
        struct in_ent *input, int str_ind, int curs_pos, int len)
{
    wmove(win, y, x);
    fill(win, x, width, '#' | A_BOLD);

    for (int i = str_ind - curs_pos;
            i - (str_ind - curs_pos) < width && i < len; i++)
        waddch(win, input->buf[i]);

    wmove(win, y, x + (curs_pos >= width ? width - 1 : curs_pos));
}


/* get user input and then validate it */
static void get_input(struct in_ent *input)
{
    init_pair(1, COLOR_RED, COLOR_BLACK);

    int inwidth = 28;

    int winx = inwidth + 2, winy = 5;
    int offx = (COLS - winx) / 2, offy = 3;
    int cury, curx;

    WINDOW *inbox = newwin(winy, winx, offy, offx);

    keypad(inbox, TRUE);
    curs_set(1);
    box(inbox, 0, 0);

    int str_ind = 0, curs_pos = 0;
    int len = input->buf[0] ? strlen(input->buf) : 0;
    draw_input(inbox, 1, 3, inwidth, input, str_ind, curs_pos, len);

    int ch;
    while ((ch = wgetch(inbox))) {
        getyx(inbox, cury, curx);
        switch (ch) {
        case KEY_LEFT:
            if (str_ind > 0) {
                if (curs_pos > 0)
                    curs_pos--;

                draw_input(inbox, 1, 3, inwidth, input, --str_ind, curs_pos, len);
            }

            break;
        case KEY_RIGHT:
            if (str_ind < len) {
                if (curs_pos < inwidth)
                    curs_pos++;

                draw_input(inbox, 1, 3, inwidth, input, ++str_ind, curs_pos, len);
            }

            break;
        case KEY_HOME:
            if (str_ind != 0 && curs_pos != 0) {
                str_ind = 0;
                curs_pos = 0;

                draw_input(inbox, 1, 3, inwidth, input, str_ind, curs_pos, len);
            }

            break;
        case KEY_END:
            if (str_ind != len && curs_pos != len % inwidth) {
                str_ind = len;
                curs_pos = len % inwidth;

                draw_input(inbox, 1, 3, inwidth, input, str_ind, curs_pos, len);
            }

            break;
        case '\n':
            if (input->validate && input->buf[0]) {
                char *err = input->validate(input->buf);

                if (err) {
                    fill(inbox, 1, inwidth, ' ');
                    wattrset(inbox, COLOR_PAIR(1));
                    mvwaddstr(inbox, 1, (inwidth - strlen(err)) / 2, err);
                    wattrset(inbox, COLOR_PAIR(0));

                    draw_input(inbox, 1, 3, inwidth, input, str_ind, curs_pos, len);
                    continue;
                }
            }

            delwin(inbox);
            curs_set(0);

            return;
        case KEY_DC:
            if (str_ind < len) {
                memmove(&input->buf[str_ind], &input->buf[str_ind + 1], len-- - str_ind);
                input->buf[len] = 0;

                draw_input(inbox, 1, 3, inwidth, input, str_ind, curs_pos, len);
            }

            break;
        case ' ' ... '~':
            if (len < input->bufsize) {
                memmove(&input->buf[str_ind + 1], &input->buf[str_ind], len++ - str_ind);
                input->buf[str_ind++] = ch;

                if (curs_pos < inwidth)
                    curs_pos++;

                draw_input(inbox, 1, 3, inwidth, input, str_ind, curs_pos, len);
            }

            break;
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
