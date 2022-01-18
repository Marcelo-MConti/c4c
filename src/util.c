#include <math.h>

#include "util.h"

size_t utf8len(const char *str)
{
    size_t len;

    for (len = 0; *str; str++)
        if ((*str & 0xc0) != 0x80) len++;

    return len;
}

void fill(WINDOW *win, int from, int to, int ch)
{
    int cury, curx;
    getyx(win, cury, curx);

    wmove(win, cury, from);
    for (int i = from; i <= to; i++)
        waddch(win, ch);
    wmove(win, cury, from);
}

void print_truncate(WINDOW *win, char *str, int len, int trunc)
{
    if (len > trunc) {
        waddnstr(win, str, trunc - 1);
        /* Ellipsis character */
        waddstr(win, "\342\200\246");
    } else {
        waddnstr(win, str, len);
    }
}

/* FIXME: Make the centering less janky */
void center_wins(struct win_off *wins, int termy, int termx)
{
    int offx, offy;
    int winx, winy;

    redrawwin(stdscr);
    wrefresh(stdscr);

    for (int i = 0; wins[i].win; i++) {
        getmaxyx(wins[i].win, winy, winx);

        offx = round(((termx - winx) / 2) + wins[i].off_offx);
        offy = round(((termy - winy) / 2) + wins[i].off_offy);

        mvwin(wins[i].win, offy, offx);

        redrawwin(wins[i].win);
        wnoutrefresh(wins[i].win);
    }

    doupdate();
}
