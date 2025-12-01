#ifndef _GAME_COMMON_H_
#define _GAME_COMMON_H_

#include <curses.h>

/*
 * Converte o índice do jogador (PLAYER_RED = 0, PLAYER_YLW = 1)
 * para o valor correspondente do tile no tabuleiro:
 *   PLAYER_RED  -> TILE_RED_CHECKER (1)
 *   PLAYER_YLW  -> TILE_YLW_CHECKER (2)
 */
#define PLAYER_TO_CHECKER(p) ((p) + 1)

/*
 * Enumeração dos jogadores disponíveis.
 */
enum player {
    PLAYER_RED,
    PLAYER_YLW
};

/*
 * Enumeração dos tipos de células (tiles) no tabuleiro.
 */
enum tile {
    TILE_NONE = 0,
    TILE_RED_CHECKER,
    TILE_YLW_CHECKER
};

/*
 * Estrutura principal que representa o estado do jogo.
 *  - board:             matriz que armazena os tiles.
 *  - blink:             matriz indicando quais tiles devem ter o efeito de vitória.
 *  - cur_player:        jogador atual (PLAYER_RED ou PLAYER_YLW).
 *  - width, height:     dimensões do tabuleiro.
 */
struct game {
    enum tile (*board)[];
    uint8_t (*blink)[];
    uint8_t cur_player;
    int width, height;
};

/*
 Representa uma posição do tabuleiro
 */
struct position {
    int x, y;
};

/*
 * Variável global modificada por um sinal SIGINT (Ctrl+C)
 * para indicar que o jogador deseja sair.
 */
extern volatile bool player_wants_to_quit;

// Verifica se uma coluna do tabuleiro ainda possui espaço disponível. Retorna
// índice da linha disponível ou -1 caso a coluna esteja cheia.
int col_is_not_full(struct game *game, int column);
// Desenha uma seta indicando a coluna selecionada no topo do tabuleiro.
void print_arrow(WINDOW *win, int idx);
// Exibe o HUD (barra de informações) na parte inferior da tela.
void print_hud(struct game *game);

#endif /* _GAME_COMMON_H_ */
