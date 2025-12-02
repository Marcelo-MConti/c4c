#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <curses.h>

#include <libintl.h>

#include "util.h"
#include "logo.h"
#include "game.h"
#include "ui/menu.h"

#define _(x) gettext(x)

#if !defined(C4C_ASCII) && !defined(C4C_COLOR)
#error "Enable either ASCII mode or color support (the game is unplayable otherwise)"
#endif

enum main_menu_entry {
    MM_ENTRY_START,
    MM_ENTRY_QUIT,
    MM_ENTRY_PLAYMODE,
    MM_ENTRY_LAST
};

/**
 * Verifica se o terminal ainda tem o tamanho mínimo após ter
 * sido redimensionado. Caso não tenha, mostra uma mensagem de
 * aviso, aguardando até que o terminal seja redimensionado novamente.
 */
void enforce_min_terminal_size() {
    while (LINES < MIN_LINES || COLS < MIN_COLS) {
        werase(stdscr);
        wnoutrefresh(stdscr);

        mvwprintw(stdscr, 1, 1, _("c4c needs at least a %dx%d terminal to work."), MIN_COLS, MIN_LINES);

        doupdate();
        wgetch(stdscr);
    }
}

// Contexto para reescrita
struct redraw_menu_ctx {
    WINDOW *logo_win;
    int win_width;
    int win_height;
};

/**
 * Função de callback chamada ao redesenhar o menu,
 * mantendo centralizado.
 *Parâmetros:
 *  menu_win:   janela do menu
    ctx:        Contexto para redesenho
 */ 
static void on_redraw_menu(WINDOW *menu_win, void *ctx)
{
    enforce_min_terminal_size();// Valida tamanho minimo terminal

    struct redraw_menu_ctx *redraw = ctx;// Contexto para redesenho

    // Calcula um novo offset da janela para deixar centralizado
    int y_offset = (LINES - redraw->win_height) / 2;
    int x_offset = (COLS - redraw->win_width) / 2;

    mvwin(redraw->logo_win, y_offset, x_offset);
    mvwin(menu_win, y_offset + ARR_SIZE(logo) + 1, x_offset);

    werase(stdscr);
    wnoutrefresh(stdscr);

    wnoutrefresh(menu_win);
    wnoutrefresh(redraw->logo_win);
}

/**
 * Função de callback chamada ao redesenhar a tela do jogo,
 * mantendo-a centralizada.
 * Parâmetros:
    gama_win:   Janela do jogo
    ctx:        contexto para redesenho     
 */
static void on_redraw_game(WINDOW *game_win, void *ctx)
{
    enforce_min_terminal_size();// Valida tamanho minimo terminal

    int width, height;
    getmaxyx(game_win, height, width); // Pega o tamanho da tela

    int y_offset = (LINES - height) / 2;
    int x_offset = (COLS - width) / 2;

    mvwin(game_win, y_offset, x_offset);

    werase(stdscr);
    wnoutrefresh(stdscr);

    wnoutrefresh(game_win);
}

int main()
{
    // Necessário para que o ncurses suporte UTF-8 e para que o gettext funcione
    setlocale(LC_ALL, "");
    setlocale(LC_CTYPE, "");

    textdomain("c4c");
    bindtextdomain("c4c", PREFIX "/share/locale");

    // Inicializa ncurses
    initscr();

#ifdef C4C_COLOR
// Verifica suporte a cores
    if (!has_colors())
        errx(1, "%s", _("This terminal does not support colors, which is required for c4c to run.\n"
                  "Either find a terminal that supports colors or recompile c4c without color support."));

    start_color();
#endif

    cbreak();
    noecho();
    curs_set(0);// esconde o cursor

    enforce_min_terminal_size(); // valida tamanho mínimo do terminal

    // número de caracteres UTF8 da maior linha da Logo
    int max_logo_width = 0;
    
    // Determina largura máxima da logo para centralizar
    for (size_t i = 0; i < ARR_SIZE(logo); i++) {
        int len = utf8len(logo[i]);
        if (len > max_logo_width)
            max_logo_width = len;
    }

    /** Entradas do menu principal do jogo, definidas declarativamente. */
    union entry_un *main_menu_entries[] = {
        [MM_ENTRY_START] = &(union entry_un) { .text = {
            ENTRY_SELECTABLE, _("START")
        }},
        [MM_ENTRY_QUIT] = &(union entry_un) { .text = {
            ENTRY_SELECTABLE, _("QUIT")
        }},
        [MM_ENTRY_PLAYMODE] = &(union entry_un) { .roulette = {
            ENTRY_ROULETTE, 0, _("PLAY MODE"),
            (char *[]) {
                [PLAY_LOCAL] = _("PL vs. PL"),
                [PLAY_LOCAL_PC] = _("PL vs. PC"),
                [PLAY_LAST] = NULL
            }
        }},
        [MM_ENTRY_LAST] = NULL
    };

    // Determina dimensões da janela do menu
    int win_height = ARR_SIZE(logo) + ARR_SIZE(main_menu_entries) - 1 + 5;
    int win_width = MAX(max_logo_width, 30);

    int y_offset = (LINES - win_height) / 2;
    int x_offset = (COLS - win_width) / 2;
    
    // Mostra a logo do jogo
    WINDOW *logo_win = newwin(ARR_SIZE(logo), win_width, y_offset, x_offset);

    int logo_width = (win_width - max_logo_width) / 2;
    
    for (int i = 0; i < ARR_SIZE(logo); i++) {
        wmove(logo_win, i, logo_width);
        waddstr(logo_win, logo[i]);
    }

    // Cria o menu
    WINDOW *menu_win = newwin(6, win_width, y_offset + ARR_SIZE(logo) + 1, x_offset);
    
    struct menu main_menu = {
        .entries = &main_menu_entries,
        .win = menu_win,
        .box = false
    };
    
    struct redraw_menu_ctx redraw_ctx = {
        .logo_win = logo_win,
        .win_height = win_height,
        .win_width = win_width,
    };

    // Loop do menu
    while (true) {
        // Redesenha o menu na tela 
        on_redraw_menu(menu_win, &redraw_ctx);
        doupdate();
        
        // Exibe o menu principal e aguarda o usuário escolher uma opção
        int entry = show_menu(&main_menu, on_redraw_menu, &redraw_ctx);

        // Trata a entrada escolhida do menu pelo usuário
        switch (entry) {
            case MM_ENTRY_START: {
                // Obtém o modo de jogo do menu
                enum play_mode mode = (*main_menu.entries)[MM_ENTRY_PLAYMODE]->roulette.cur_option;

                struct game_params params = {
                    .width = DEFAULT_WIDTH,
                    .height = DEFAULT_HEIGHT,
                    .mode = mode
                };

                start_game(&params, on_redraw_game, NULL);

                break;
            }
            case -1:
            case MM_ENTRY_QUIT:  // Fecha o jogo
                nocbreak();
                echo();
                curs_set(1);
                endwin();

                return 0;
        }
    }
}
