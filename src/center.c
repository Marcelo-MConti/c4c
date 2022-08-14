#include <err.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include <curses.h>

struct center_wins {
    struct center_win_info {
        WINDOW *win;
        int z_index;
        int y_off, x_off;
    } *info;
    size_t used, cap;
} center;

void (*ncurses_sig_handler)(int);
void (*ncurses_siginfo_sig_handler)(int, siginfo_t *, void *);
bool ncurses_uses_siginfo;


// FIXME: Figure out why this is so janky
void center_win_trigger(void)
{
    int termx, termy;
    getmaxyx(curscr, termy, termx);

    redrawwin(stdscr);
    wnoutrefresh(stdscr);

    for (size_t i = center.used; i-- > 0;) {
        double y, x;
        getmaxyx(center.info[i].win, y, x);

        y = round((termy - y) / 2) + center.info[i].y_off;
        x = round((termx - x) / 2) + center.info[i].x_off;

        mvwin(center.info[i].win, y, x);
        redrawwin(center.info[i].win);
        wnoutrefresh(center.info[i].win);
    }

    doupdate();
}

static void winch_move_wins(int signo, siginfo_t *info, void *context)
{
    if (!ncurses_sig_handler && !ncurses_siginfo_sig_handler)
        err(1, "Function pointer was NULL");

    if (ncurses_uses_siginfo)
        ncurses_siginfo_sig_handler(signo, info, context);
    else
        ncurses_sig_handler(signo);

    center_win_trigger();
}

void center_init(void)
{
    center.cap = 16;
    center.info = malloc(sizeof(struct center_win_info) * center.cap);

    if (!center.info)
        err(1, "Failed to allocate memory!");

    struct sigaction oldact;
    sigaction(SIGWINCH, NULL, &oldact);

    ncurses_uses_siginfo = oldact.sa_flags;
    if (ncurses_uses_siginfo)
        ncurses_siginfo_sig_handler = oldact.sa_sigaction;
    else
        ncurses_sig_handler = oldact.sa_handler;

    const struct sigaction act = {
        .sa_sigaction = winch_move_wins,
        .sa_flags = SA_SIGINFO | SA_RESTART,
        .sa_mask = oldact.sa_mask
    };

    sigaction(SIGWINCH, &act, NULL);
}

void center_win_add(WINDOW *win, int z_index, int y_off, int x_off)
{
    if (center.used == center.cap) {
        center.cap *= 2;
        struct center_win_info *tmp = realloc(center.info, sizeof(struct center_win_info) * center.cap);
        if (!tmp)
            err(1, "Failed to allocate memory!");
        center.info = tmp;
    }

    size_t i;
    for (i = 0; i < center.used && z_index <= center.info[i].z_index; i++);

    if (i < center.used)
        memmove(&center.info[i + 1], &center.info[i], center.used - i);

    center.info[i].win = win;
    center.info[i].z_index = z_index;
    center.info[i].y_off = y_off;
    center.info[i].x_off = x_off;
    center.used++;
}

void center_win_remove(WINDOW *win)
{
    for (size_t i = 0; i < center.used; i++) {
        if (center.info[i].win == win) {
            if (i < center.used)
                memmove(&center.info[i], &center.info[i + 1], center.used - i);

            center.used--;
            return;
        }
    }
}
