#ifndef _GAME_LOCAL_H_
#define _GAME_LOCAL_H_

#include <curses.h>

#include "game/common.h"
/*
 Permite que o jogador local escolha uma coluna para jogar.
 Retorna a posicao selecionada pelo jogador
 Parãmetros:
    Parâmetros:
    win: janela ncurses onde o tabuleiro é desenhado.
    game: Estrutura do jogo contendo o tabuleiro e o jogador atual.
    on_redraw: Função para redesenhar a tela quando necessário.
    ctx: Contexto a ser passado para a função de redraw.
 */
struct position local_play(WINDOW *win, struct game *game, void (*on_redraw)(WINDOW *, void *ctx), void *ctx);

#endif /* _GAME_LOCAL_H_ */
