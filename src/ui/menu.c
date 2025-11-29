#include <curses.h>
#include <stdio.h>
#include <string.h>

#include <err.h>

#include "util.h"
#include "ui/menu.h"

enum {
    PREVIOUS,
    NEXT
};

/** Desenha uma entrada do menu (cada entrada ocupa uma linha) */
static bool draw_entry(struct menu *menu, int index)
{
    int winx;
    int cury, curx;
    getmaxyx(menu->win, (int){0}, winx);
    getyx(menu->win, cury, curx);

    union entry_un *ent = (*menu->entries)[index];

    if (ent->common.type == ENTRY_CONDITIONAL) {
        if (ent->conditional.condition(menu))
            ent = ent->conditional.entry;
        else
            return false;
    }

    if (index == menu->cur_entry)
        wattrset(menu->win, A_REVERSE);

    fill(menu->win, curx, winx - 2, ' ');
    wmove(menu->win, cury, curx);

    switch (ent->common.type) {
        case ENTRY_TEXT:
        case ENTRY_SELECTABLE: ;
            /* -2: bordas */
            int newx = winx - strlen(ent->text.text) - 2;
            mvwaddstr(menu->win, cury, newx, ent->text.text);

            break;
        case ENTRY_ROULETTE:
            wprintw(menu->win, " %s: ", ent->roulette.text);
            waddstr(menu->win, "< ");

            int len = strlen(ent->roulette.alt[ent->roulette.cur_option]);

            /* -1: borda, -2: "> " no final */
            mvwaddstr(menu->win, cury, winx - 1 - 2 - len,
                    ent->roulette.alt[ent->roulette.cur_option]);
            waddstr(menu->win, "> ");
            break;
        case ENTRY_INPUT:
            mvwprintw(menu->win, cury, 1, " %s: ", ent->input.text);

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

    wattrset(menu->win, A_NORMAL);

    return true;
}

/** Retorna o índice da entrada selecionada */ 
static int draw_menu(struct menu *menu)
{
    wclrtobot(menu->win);

    if (menu->box)
        box(menu->win, 0, 0);

    wmove(menu->win, 1, 1);

    int cury, curx, tmp = -1;

    for (int i = 0; (*menu->entries)[i]; i++) {
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

/** Retorna o índice da próxima/última entrada visível no menu */
static int seek_to_entry(struct menu *menu, int whence)
{
    int inc = menu->cur_entry;

    while (true) {
        union entry_un *ent;

        if (whence == PREVIOUS && inc == 0)
            return 0;

        ent = (*menu->entries)[whence == PREVIOUS ? --inc : ++inc];

        if (whence == NEXT && !ent)
            return menu->cur_entry;

        if (ent->common.type == ENTRY_CONDITIONAL && !ent->conditional.condition(menu))
            continue;

        return inc;
    }
}

/** Desenha a barra de entrada */
static void draw_input(WINDOW *win, int x, int y, int width,
        struct in_ent *input, int str_idx, int curs_pos, int len)
{
    wmove(win, y, x);
    fill(win, x, width, ' ' | A_REVERSE | A_DIM);

    for (int i = str_idx - curs_pos;
            i - (str_idx - curs_pos) < width && i < len; i++)
        waddch(win, input->buf[i] | A_REVERSE | A_DIM);

    wmove(win, y, x + (curs_pos >= width ? width - 1 : curs_pos));
}


/** Lê uma entrada do usuário e chama a função de validação */
static void get_input(struct in_ent *input, void (*on_redraw)(WINDOW *, void *), WINDOW *menu_win, void *ctx)
{
    init_pair(1, COLOR_RED, COLOR_BLACK);

    int inwidth = 30;

    int winx = inwidth + 2, winy = 5;
    int offx = (COLS - winx) / 2, offy = (LINES - winy) / 2; 
    int curx;

    WINDOW *inbox = newwin(winy, winx, offy, offx);

    keypad(inbox, TRUE);
    curs_set(1);
    box(inbox, 0, 0);

    mvwaddstr(inbox, 1, 1, input->description);

    int str_idx = 0, curs_pos = 0;
    int len = input->buf[0] ? strlen(input->buf) : 0;
    draw_input(inbox, 1, 3, inwidth, input, str_idx, curs_pos, len);

    bool redraw = false;

    int ch;
    while ((ch = wgetch(inbox))) {
        getyx(inbox, (int){0}, curx);

        if (redraw) {
            refresh();
            redraw = false;            
        }
        
        switch (ch) {
            case KEY_LEFT:
                if (str_idx > 0) {
                    if (curs_pos > 0)
                        curs_pos--;

                    draw_input(inbox, 1, 3, inwidth, input, --str_idx, curs_pos, len);
                }

                break;
            case KEY_RIGHT:
                if (str_idx < len) {
                    if (curs_pos < inwidth)
                        curs_pos++;

                    draw_input(inbox, 1, 3, inwidth, input, ++str_idx, curs_pos, len);
                }

                break;
            case KEY_HOME:
                if (str_idx != 0 && curs_pos != 0) {
                    str_idx = 0;
                    curs_pos = 0;

                    draw_input(inbox, 1, 3, inwidth, input, str_idx, curs_pos, len);
                }

                break;
            case KEY_END:
                if (str_idx != len && curs_pos != len % inwidth) {
                    str_idx = len;
                    curs_pos = len % inwidth;

                    draw_input(inbox, 1, 3, inwidth, input, str_idx, curs_pos, len);
                }

                break;
            case '\n':
            case KEY_ENTER:
                if (input->validate && input->buf[0]) {
                    const char *err = input->validate(input->buf);

                    if (err) {
                        wattrset(stdscr, COLOR_PAIR(1));
                        mvwaddstr(stdscr, LINES - 2, (COLS - strlen(err)) / 2, err);
                        wattrset(stdscr, COLOR_PAIR(0));

                        redraw = true;

                        draw_input(inbox, 1, 3, inwidth, input, str_idx, curs_pos, len);
                        continue;
                    }
                }

                delwin(inbox);
                curs_set(0);

                return;
            case KEY_DC:
                if (str_idx < len) {
                    memmove(&input->buf[str_idx], &input->buf[str_idx + 1], len-- - str_idx);
                    input->buf[len] = 0;

                    draw_input(inbox, 1, 3, inwidth, input, str_idx, curs_pos, len);
                }

                break;
            // Caracteres ASCII: alfanuméricos + símbolos
            case ' ' ... '~':
                if (len < input->bufsize) {
                    memmove(&input->buf[str_idx + 1], &input->buf[str_idx], len++ - str_idx);
                    input->buf[str_idx++] = ch;

                    if (curs_pos < inwidth)
                        curs_pos++;

                    draw_input(inbox, 1, 3, inwidth, input, str_idx, curs_pos, len);
                }

                break;
            // Várias formas diferentes de representar backspace...
            // Espero que uma delas funcione no seu terminal.
            case KEY_BACKSPACE:
            case '8' & 0x1f:
            case '?' & 0x1f:
            case 127:
            case '\b':
                if (str_idx > 0 && curs_pos > 0) {
                    memmove(&input->buf[str_idx - 1], &input->buf[str_idx], len-- - str_idx + 1);
                    input->buf[len] = 0;

                    draw_input(inbox, 1, 3, inwidth, input, --str_idx, --curs_pos, len);
                }
            case KEY_RESIZE:
                on_redraw(menu_win, ctx);

                offx = (COLS - winx) / 2;
                offy = (LINES - winy) / 2;
                
                mvwin(inbox, offy, offx);
                draw_input(inbox, 1, 3, inwidth, input, str_idx, curs_pos, len);

                touchwin(inbox);
                wsyncup(inbox);

                wnoutrefresh(stdscr);
                wnoutrefresh(inbox);

                doupdate();

                break;
        }
    }

    reset_color_pairs();
}

int run_menu(struct menu *menu, void (*on_redraw)(WINDOW *, void *ctx), void *ctx)
{
    if (!menu || !menu->entries)
        return -1;

    if (menu->box)
        box(menu->win, 0, 0);

    keypad(menu->win, TRUE);
    wmove(menu->win, 1, 1);

    draw_menu(menu);

    WINDOW *parent = wgetparent(menu->win);

    int ch;
    while ((ch = wgetch(menu->win)) != ERR) {
        union entry_un *ent = (*menu->entries)[menu->cur_entry];

        if (ent->common.type == ENTRY_CONDITIONAL)
            ent = ent->conditional.entry;

        wnoutrefresh(parent);
        wnoutrefresh(stdscr);

        doupdate();

        switch (ch) {
            case KEY_UP: ;
                int prev_ent = seek_to_entry(menu, PREVIOUS);

                if (menu->cur_entry != prev_ent) {
                    menu->cur_entry = prev_ent;

                    wmove(menu->win, 1, 1);
                    int vis_cur_y = draw_menu(menu);
                    wmove(menu->win, vis_cur_y - 1, 1);
                }

                break;
            case KEY_DOWN: ;
                int next_ent = seek_to_entry(menu, NEXT);

                if (menu->cur_entry != next_ent) {
                    menu->cur_entry = next_ent;

                    wmove(menu->win, 1, 1);
                    int vis_cur_y = draw_menu(menu);
                    wmove(menu->win, vis_cur_y, 1);
                }

                break;
            case KEY_LEFT:
                if (ent->common.type == ENTRY_ROULETTE) {
                    struct roul_ent *tmp = &ent->roulette;

                    if (tmp->cur_option != 0) {
                        tmp->cur_option--;

                        wmove(menu->win, 1, 1);
                        int vis_cur_y = draw_menu(menu);
                        wmove(menu->win, vis_cur_y, 1);
                    }
                }

                break;
            case KEY_RIGHT:
                if (ent->common.type == ENTRY_ROULETTE) {
                    struct roul_ent *tmp = &ent->roulette;

                    if (tmp->alt[tmp->cur_option + 1]) {
                        tmp->cur_option++;

                        wmove(menu->win, 1, 1);
                        int vis_cur_y = draw_menu(menu);
                        wmove(menu->win, vis_cur_y, 1);
                    }
                }

                break;
            case '\n':
            case KEY_ENTER:
            case ' ':
                switch (ent->common.type) {
                    case ENTRY_SELECTABLE:
                        return menu->cur_entry;
                    case ENTRY_CONDITIONAL:
                        return -1;
                    case ENTRY_INPUT:
                        get_input(&ent->input, on_redraw, menu->win, ctx);

                        draw_menu(menu);
                        on_redraw(menu->win, ctx);
                    default:
                        putchar('\a');
                }

                break;
            case KEY_RESIZE:
                on_redraw(menu->win, ctx);
        }

        touchwin(menu->win);
        wsyncup(menu->win);

        wnoutrefresh(menu->win);
        wnoutrefresh(parent);

        doupdate();
    }

    return -1;
}
