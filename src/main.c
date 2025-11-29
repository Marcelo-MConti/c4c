#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <curses.h>

#include "util.h"
#include "logo.h"
#include "game.h"
#include "ui/menu.h"

#if !defined(C4C_ASCII) && !defined(C4C_COLOR)
#error "Enable either ASCII mode or color support (the game is unplayable otherwise)"
#endif

enum main_menu_entry {
    MM_ENTRY_START,
    MM_ENTRY_QUIT,
    MM_ENTRY_PLAYMODE,
    MM_ENTRY_NETPLAY_HOST,
    MM_ENTRY_LAST
};

/**
 * Valida e faz parsing de uma string especificando um host e uma porta (`buf`).
 * O host pode ser um nome de domínio, um endereço IPv4 ou um endereço IPv6.
 *
 * Retorna `NULL` se a string for válida e uma mensagem de erro se houve um erro
 * de validação/parsing.
 *
 * Se `host` e `port` forem diferentes de `NULL`, guarda em `*host` e `*port` o
 * host e a porta lidos. `*host` é alocado dinamicamente e deve ser desalocado
 * por quem chamou.
 */
static const char *validate_split_host_port(char *buf, char **host, uint16_t *port)
{
    static const char *invalid_v6 = "Invalid IPv6 address! Expected `[ADDR]:PORT' or just `ADDR'.";
    static const char *invalid_host = "Invalid host! `HOST:PORT' or just `HOST' expected. `HOST' can be a domain name or IPv4/6 address.";

    char *colon = strchr(buf, ':');
    char *v6end = NULL;
    
    // Assumir que é um endereço IPv6 se usar dois "dois pontos" ou colchetes
    if (colon && strchr(colon + 1, ':')) {
        colon = v6end = strchr(buf, '\0');
    } else if (buf[0] == '[') {
        v6end = strchr(buf, ']');
        colon = v6end + 1;

        if (*colon != ':') 
            return invalid_v6;
    } else {
        for (char *s = buf; *s != ':' && *s != '\0'; s++) {
            if (!isalnum(*s) && *s != '.' && *s != '-')
                return invalid_host;
        }
    }

    if (v6end) {
        for (char *s = buf; s != v6end; s++) {
            if (*s != ':' && !isdigit(*s) && tolower(*s) < 'a' && tolower(*s) > 'f')
                return invalid_v6;
        }
    }

    unsigned long parsed_port = 0;

    if (!colon || *colon == '\0')
        goto valid_host;

    parsed_port = strtoul(colon + 1, NULL, 10);

    if (parsed_port <= 1024 || parsed_port >= 65536)
        goto valid_host;
    else
        return invalid_host;

valid_host:
    if (port)
        *port = parsed_port ?: DEFAULT_PORT; 

    if (host) {
        char *host_start = &buf[0];
        size_t host_len;
        
        if (v6end) {
            if (*v6end == ']')
                host_start++;

            host_len = v6end - host_start;
        } else {
            host_len = colon - buf;
        }

        *host = malloc(host_len + 1);
        memcpy(*host, buf, host_len);

        (*host)[host_len] = '\0';
    }

    return NULL;
}

/**
 * Usada como callback para validar o campo de entrada `HOST` no menu.
 * vd. `validate_split_host`.
 */ 
const char *menu_validate_host(char *buf)
{
    return validate_split_host_port(buf, NULL, NULL);
}

/** Verifica se a opção netplay está selecionada no menu. */ 
static bool menu_condition_netplay(struct menu *menu)
{
    struct roul_ent *play_mode = &(*menu->entries)[MM_ENTRY_PLAYMODE]->roulette;
    return play_mode->cur_option == PLAY_NET;
}

/** Entradas do menu principal do jogo, definidas declarativamente. */
static union entry_un *main_menu_entries[] = (union entry_un *[]) {
    [MM_ENTRY_START] = &(union entry_un) { .text = {
        ENTRY_SELECTABLE, "START"
    }},
    [MM_ENTRY_QUIT] = &(union entry_un) { .text = {
        ENTRY_SELECTABLE, "QUIT"
    }},
    [MM_ENTRY_PLAYMODE] = &(union entry_un) { .roulette = {
        ENTRY_ROULETTE, 0, "PLAY MODE",
        (char *[]) {
            [PLAY_LOCAL] = "PL vs. PL",
            [PLAY_LOCAL_PC] = "PL vs. PC",
            [PLAY_NET] = "NETPLAY",
            [PLAY_LAST] = NULL
        }
    }},
    [MM_ENTRY_NETPLAY_HOST] = &(union entry_un) { .conditional = {
        ENTRY_CONDITIONAL,
        &(union entry_un) { .input = {
            ENTRY_INPUT, 255, "HOST", (char [256]) {0}, "Hostname or address of peer:", menu_validate_host
        }},
        menu_condition_netplay
    }},
    [MM_ENTRY_LAST] = NULL
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

        // NOTE: update article if `MIN_COLS` changes
        static const char msg[] = "c4c needs at least an " STR(MIN_COLS) "x" STR(MIN_LINES) " terminal to work.";

        mvwaddstr(stdscr, 1, 1, msg);

        doupdate();
        wgetch(stdscr);
    }
}

struct redraw_menu_ctx {
    WINDOW *logo_win;
    int win_width;
    int win_height;
};

/**
 * Função de callback chamada ao redesenhar o menu,
 * mantendo-o centralizado.
 */ 
static void on_redraw_menu(WINDOW *menu_win, void *ctx)
{
    enforce_min_terminal_size();

    struct redraw_menu_ctx *redraw = ctx;

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
 */
static void on_redraw_game(WINDOW *game_win, void *ctx)
{
    enforce_min_terminal_size();

    int width, height;
    getmaxyx(game_win, height, width);

    int y_offset = (LINES - height) / 2;
    int x_offset = (COLS - width) / 2;

    mvwin(game_win, y_offset, x_offset);

    werase(stdscr);
    wnoutrefresh(stdscr);

    wnoutrefresh(game_win);
}

int main()
{
    // Necessário para que o ncurses suporte UTF-8 em algumas versões
    setlocale(LC_CTYPE, "");
    initscr();

#ifdef C4C_COLOR
    if (!has_colors())
        errx(1, "This terminal does not support colors, which is required for c4c to run.\n"
                "Either find a terminal that supports colors or recompile c4c without color support.");

    start_color();
#endif

    cbreak();
    noecho();
    curs_set(0);

    enforce_min_terminal_size();

    int max_logo_width = 0;
    
    for (size_t i = 0; i < ARR_SIZE(logo); i++) {
        int len = utf8len(logo[i]);
        if (len > max_logo_width)
            max_logo_width = len;
    }

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

    while (true) {
        on_redraw_menu(menu_win, &redraw_ctx);
        doupdate();
        
        int entry = run_menu(&main_menu, on_redraw_menu, &redraw_ctx);

        switch (entry) {
            case MM_ENTRY_START: {
                // Obtém o modo de jogo e o host (usado se for netplay) do menu
                enum play_mode mode = (*main_menu.entries)[MM_ENTRY_PLAYMODE]->roulette.cur_option;
                char *host = (*main_menu.entries)[MM_ENTRY_NETPLAY_HOST]->conditional.entry->input.buf;

                if (mode == PLAY_NET && *host == '\0') {
                    init_pair(1, COLOR_RED, COLOR_BLACK);

                    static const char err[] = "You need to specify a peer (`HOST:PORT' or just `HOST') in netplay mode.";

                    wattrset(stdscr, COLOR_PAIR(1));
                    mvwaddstr(stdscr, LINES - 2, (COLS - sizeof(err) - 1) / 2, err);
                    wattrset(stdscr, COLOR_PAIR(0));
                    wgetch(stdscr);
                    
                    reset_color_pairs();

                    continue;
                }
                
                start_game(DEFAULT_WIDTH, DEFAULT_HEIGHT, mode, on_redraw_game, NULL);

                break;
            }
            case -1:
            case MM_ENTRY_QUIT:
                nocbreak();
                echo();
                curs_set(1);
                endwin();

                return 0;
        }
    }
}
