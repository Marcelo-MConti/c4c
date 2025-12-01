#include <stdlib.h>
#include <curses.h>

#include <libintl.h>

#include "game/common.h"
#include "ui/yn_prompt.h"

#define _(x) gettext(x)

/**
 Permite que o jogador local escolha uma coluna para jogar.
 
 Esta função mostra uma seta sobre a coluna selecionada e permite que o jogador
 mova a seta usando as teclas, confirmando a jogada com Enter ou espaço. 

 Parâmetros:
    win: janela ncurses onde o tabuleiro é desenhado.
    game: Estrutura do jogo contendo o tabuleiro e o jogador atual.
    on_redraw: Função para redesenhar a tela quando necessário.
    ctx: Contexto a ser passado para a função de redraw.
 
 Retorna:
    struct position com a posição selecionada pelo jogador.
 */
struct position local_play(WINDOW *win, struct game *game, void (*on_redraw)(WINDOW *, void *ctx), void *ctx)
{
    static struct position pos = {0};
    int idx = pos.x, ch, tmp;

    wmove(win, 0, 1);
    print_arrow(win, idx);

    while (true) {
       if (player_wants_to_quit) {
        // Mostra na tela as opções de yes or no para sair do jogo
            player_wants_to_quit = show_yn_prompt(_("Do you really want to quit?"), YN_LABEL_YES_NO, on_redraw, ctx);

            if (player_wants_to_quit) {// Finaliza o jogo caso o jogador queira
                nocbreak();
                echo();
                curs_set(1);
                endwin();

                exit(0);
            }

            on_redraw(win, NULL);// Caso não queira sair redesenha a tela

            print_hud(game);
            doupdate();
        }

        ch = wgetch(win);
        
        // Checa a tecla pressionada pelo usuário e movimenta a seta de acordo com isso
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
            case KEY_ENTER:// Caso aperte enter é escolhido a posição da jogada
            case ' ':
                if ((tmp = col_is_not_full(game, idx)) != -1) {
                    pos.x = idx;
                    pos.y = tmp;

                    wmove(win, 0, 1);
                    wclrtoeol(win);

                    return pos;
                } else {
                    // XXX: show error message
                }

                break;
            case KEY_RESIZE:// Redimensiona a pagina caso mudemos o tamanho da tela
                on_redraw(win, ctx);

                print_hud(game);
                doupdate();

                break;
            case '1' ... '7':
                idx = ch - '1';

                wmove(win, 0, 1);
                print_arrow(win, idx);

                break;
        }
    }
}
