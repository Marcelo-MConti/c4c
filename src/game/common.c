#include <curses.h>

#include <libintl.h>

#include "util.h"
#include "chars.h"
#include "game/common.h"

#define _(x) gettext(x)

/**
 Verifica se uma coluna do tabuleiro ainda possui espaço disponível.
 Parâmetros:
    game   - Ponteiro para a estrutura do jogo contendo o tabuleiro.
    column - Índice da coluna a ser verificada.
  
 Retorno: índice da linha disponível ou -1 caso a coluna esteja cheia.
 */
int col_is_not_full(struct game *game, int column)
{
    enum tile (*board)[game->width] = game->board;

    for (int i = 0; i < game->height; i++) {
        if (board[i][column] == TILE_NONE)
            return i;
    }

    return -1;
}

/**
  Desenha uma seta indicando a coluna selecionada no topo do tabuleiro.
 
 Parâmetros:
 win    -Janela curses onde a seta será desenhada.
 ind    -Índice da coluna atualmente selecionada.
 */


void print_arrow(WINDOW *win, int ind)
{
    int curx, cury;
    getyx(win, cury, curx);

    wclrtoeol(win);
    mvwaddstr(win, cury, ind * 3 + 1, arrow);
}

/**
 Exibe o HUD (barra de informações) na parte inferior da tela.
 
  Esta função desenha a interface informativa que inclui:
 *  - Indicação visual do jogador atual, destacando a peça correspondente
 *    com colchetes ao redor.
 *  - Ícones das peças usando suas cores configuradas.
 *  - A mensagem de instruções alinhada à direita.
  Parâmetros:
    *game:    Estrutura principal que representa o estado do jogo.  
 */
void print_hud(struct game *game)
{
    const char *hud_msg = _("(^C) Quit     (<-/->/Home/End) Move arrow     (Enter) Play");

    wmove(stdscr, LINES - 1, 1);

    waddch(stdscr, game->cur_player == PLAYER_RED ? '[' : ' ');
    
    wattrset(stdscr, COLOR_PAIR(TILE_RED_CHECKER));
    waddstr(stdscr, checkers[TILE_RED_CHECKER]);
    wattrset(stdscr, COLOR_PAIR(0));
    
    waddch(stdscr, game->cur_player == PLAYER_RED ? ']' : ' ');

    waddch(stdscr, game->cur_player == PLAYER_YLW ? '[' : ' ' | COLOR_PAIR(TILE_YLW_CHECKER));

    wattrset(stdscr, COLOR_PAIR(TILE_YLW_CHECKER));
    waddstr(stdscr, checkers[TILE_YLW_CHECKER]);
    wattrset(stdscr, COLOR_PAIR(0));

    waddch(stdscr, game->cur_player == PLAYER_YLW ? ']' : ' ' | COLOR_PAIR(0));

    mvwaddstr(stdscr, LINES - 1, COLS - utf8len(hud_msg) - 1, hud_msg);

    wnoutrefresh(stdscr);
} 
