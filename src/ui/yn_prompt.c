#include <string.h>
#include <curses.h>

#include "ui/yn_prompt.h"

#define N 0
#define Y 1

#define MAX_LABELS 2

const char *labels[][MAX_LABELS] = {
    [YN_LABEL_YES_NO] = { "No", "Yes" },
    [YN_LABEL_OK_CANCEL] = { "Cancel", "OK" }
};

bool show_yn_prompt(char *text, enum yn_prompt_label type, void (*on_redraw)(WINDOW *, void *ctx), void *ctx)
{
    const int width = 35;
    const int height = 5;
    WINDOW *win = newwin(height, width, 1, 1);

    box(win, 0, 0);
    keypad(win, TRUE);

    size_t y_label_len = strlen(labels[type][Y]);
    size_t n_label_len = strlen(labels[type][N]);

    size_t total_len = y_label_len + n_label_len + 4;

    size_t space_around = (width - 2 - total_len) / 3;
    size_t space_between = width - 2 - total_len - 2 * space_around;

    int selected = -1;

    mvwaddstr(win, 1, 2, text);

    bool first = true;
    int ch;

    while (true) {
        ch = first ? 0 : wgetch(win);
        
        switch (ch) {
            case KEY_LEFT:
                if (selected == -1 || selected == 0)
                    break;

                selected--;
                break;
            case KEY_RIGHT:
                if (selected == -1 || selected == MAX_LABELS - 1)
                    break;

                selected++;
                break;
            case '\t':
                selected = (selected + 1) % 2;
                break;
            case '\n':
            case KEY_ENTER:
                if (selected != -1)
                    return selected;

                break;
            case KEY_RESIZE:
                on_redraw(win, ctx);
                break;
        }

        wmove(win, 3, space_around);

        if (selected == N)
            wattrset(win, A_REVERSE);

        waddch(win, '<');
        waddstr(win, labels[type][N]);
        waddch(win, '>');

        wattrset(win, A_NORMAL);

        wmove(win, 3, space_around + n_label_len + 2 + space_between);

        if (selected == Y)
            wattrset(win, A_REVERSE);
 
        waddch(win, '<');
        waddstr(win, labels[type][Y]);
        waddch(win, '>');

        wattrset(win, A_NORMAL);

        if (first)
            on_redraw(win, NULL);

        touchwin(win);
        wsyncup(win);

        wnoutrefresh(win);
        wnoutrefresh(stdscr);

        doupdate();

        first = false;
    }

    return selected;
}
