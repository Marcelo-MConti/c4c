#include <curses.h>

#include "game.h"
#include "game/common.h"

struct position *local_play(WINDOW *win, struct game *game, void (*on_redraw)(WINDOW *, void *ctx), void *ctx)
{
    static struct position pos = {0};
    int idx = pos.x, ch, tmp;

    wmove(win, 0, 1);
    print_arrow(win, idx);

    while (true) {
        ch = wgetch(win);
        
        switch (ch) {
            case KEY_LEFT:
                if (idx != 0) {
                    idx--;

                    wmove(win, 0, 1);
                    print_arrow(win, idx);
                }

                break;
            case KEY_RIGHT:
                if (idx != game->width - 1) {
                    idx++;

                    wmove(win, 0, 1);
                    print_arrow(win, idx);
                }

                break;
            case KEY_HOME:
                idx = 0;

                wmove(win, 0, 1);
                print_arrow(win, idx);

                break;
            case KEY_END:
                idx = game->width - 1;

                wmove(win, 0, 1);
                print_arrow(win, idx);

                break;
            case '\n':
            case KEY_ENTER:
            case ' ':
                if ((tmp = col_is_not_full(game, idx)) != -1) {
                    pos.x = idx;
                    pos.y = tmp;

                    wmove(win, 0, 1);
                    wclrtoeol(win);

                    return &pos;
                } else {
                    // XXX: show error message
                }

                break;
            case KEY_RESIZE:
                on_redraw(win, ctx);
                doupdate();
                break;
            case '1' ... '7':
                idx = ch - '1';

                wmove(win, 0, 1);
                print_arrow(win, idx);
        }
    }
}
