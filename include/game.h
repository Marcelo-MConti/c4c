#ifndef _GAME_H_
#define _GAME_H_

#include <netdb.h>
#include <stdint.h>
#include <curses.h>

/*
 Declarações gerais do módulo principal do jogo.
 
  Constantes:
    DEFAULT_PORT:
 *     Porta padrão usada para partidas em rede.
 
    DEFAULT_WIDTH / DEFAULT_HEIGHT:
 *     Dimensões padrão do tabuleiro (largura e altura).
 
  enum play_mode:
    Define os modos de jogo disponíveis:
 *     - PLAY_LOCAL: Dois jogadores no mesmo dispositivo.
 *     - PLAY_LOCAL_PC: Jogador contra o computador.
 *     - PLAY_NET: Partida via rede.
 *     - PLAY_LAST: Marcador auxiliar (não representa um modo jogável).

  start_game():
    Função responsável por inicializar e gerenciar a execução de uma partida.
    Parâmetros:
 *     - params: Parâmetros do jogo (dimensões do tabuleiro, hostname, etc.)
 *     - on_redraw: Callback chamado sempre que a interface deve ser redesenhada.
 *     - ctx: Contexto opcional passado para o callback.
 */


#define DEFAULT_PORT 36540

#define DEFAULT_WIDTH 7
#define DEFAULT_HEIGHT 6

enum play_mode {
    PLAY_LOCAL,
    PLAY_LOCAL_PC,
    PLAY_NET,
    PLAY_LAST
};

struct game_params {
    int width;
    int height;
    enum play_mode mode;
    char *host;
    uint16_t port;
};

void start_game(struct game_params *params, void (*on_redraw)(WINDOW *, void *ctx), void *ctx);

#endif /* _GAME_H_ */
