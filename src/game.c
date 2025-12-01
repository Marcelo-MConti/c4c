#include <string.h>
#include <stdlib.h>
#include <curses.h>
#include <signal.h>
#include <unistd.h>

#include <libintl.h>

#include "game.h"
#include "game/common.h"

#include "game/local.h"
#include "game/ia.h"

#include "chars.h"


#define ERROR_COLOR RED_CHECKER
#define WARN_COLOR  YLW_CHECKER

#define _(x) gettext(x)
#define N_(x) x


/**
 * Esse array é ordenado de forma que as posições `i` e `i + 4`
 * sejam inversas uma da outra. Isso simplifica a lógica da função
 * `check_win`.
 */
static const struct position neighbour_pos[] = {
    { 0,  1 }, {  1,  1 }, {  1,  0 }, {  1, -1 },
    { 0, -1 }, { -1, -1 }, { -1,  0 }, { -1,  1 }
};

static const char *end_messages[][2] = {
    [PLAY_LOCAL] = { N_("Red won!"), N_("Yellow won!") },
    [PLAY_LOCAL_PC] = { N_("You won."), N_("The computer won.") }
};

/**
  Verifica se uma posição no tabuleiro é válida e contém uma peça.

  Parâmetros:
    - game: Ponteiro para o estado atual do jogo (contém o tabuleiro e dimensões).
    - pos:  Posição a ser verificada (coordenadas x e y).
  Retorno:
    - true:  se a posição está dentro dos limites e contém uma peça.
    - false: caso contrário (fora do tabuleiro ou posição vazia).
 */
static inline bool is_valid_nonempty_pos(struct game *game, struct position *pos)
{
    if ((pos->x < 0 || pos->y < 0) || (pos->x >= game->width || pos->y >= game->height))
        return false;

    enum tile (*board)[game->width] = game->board;
    return board[pos->y][pos->x] != TILE_NONE;
}

/**
  Verifica se a última jogada gerou uma condição de vitória.
 
  A função analisa a peça recém-colocada em `pos` e verifica, para cada uma das
  quatro direções principais (horizontal, vertical e as duas diagonais), quantas
  peças iguais existem consecutivamente a partir dela.
 
  Retorno: Ponteiro para um array estático `uint8_t[4]` contendo as quantidades
 *         de peças iguais em cada eixo (horizontal, vertical e diagonais),
 *         caso exista alguma direção vencedora; ou NULL caso não haja vitória.
 */

static uint8_t (*check_win(struct game *game, struct position *pos))[4]
{
    // Ponteiro para o tabuleiro do jogo
    enum tile (*board)[game->width] = game->board;

    // Se a posição ainda estiver vazia, não há vitória possível
    if (board[pos->y][pos->x] == TILE_NONE)
        return NULL;

    // Array estático para contar peças iguais em cada uma das 4 direções principais
    static uint8_t same_neighbours[4];
    memset(same_neighbours, 0, sizeof same_neighbours);

    // Itera pelas direções
    for (int i = 0; i < 8; i++) {
        struct position chk_pos = { pos->x, pos->y };

        // Verifica até 3 casas à frente na direção atual
        for (int j = 0; j < 4; j++) { 
            chk_pos.x += neighbour_pos[i].x;
            chk_pos.y += neighbour_pos[i].y;

             // Se a posição for válida e tiver a mesma peça, incrementa contador
            if (is_valid_nonempty_pos(game, &chk_pos) &&
                    board[pos->y][pos->x] == board[chk_pos.y][chk_pos.x])
                same_neighbours[i % 4]++;
            else
                break;// interrompe se encontrar peça diferente ou posição inválida
        }
    }

    for (int i = 0; i < 4; i++) {
        if (same_neighbours[i] >= 3)
            return &same_neighbours;// retorna array com contagens
    }

    return NULL; // Caso não haja vitória retorna NULL
}

/**
  Desenha o tabuleiro do jogo na janela fornecida.
  Mostra as peças de cada posição com cores, aplica efeito de piscar
  nas peças destacadas e desenha os separadores entre as casas.
  
  Parâmetros:
 *  -win  A janela ncurses onde o tabuleiro será desenhado.
 *  -game Estrutura do jogo contendo o tabuleiro, dimensões e efeitos de blink.
 */
static void print_board(WINDOW *win, struct game *game)
{
    int curx, cury;
    int winx, winy;

    getmaxyx(win, winy, winx);// Obtém o tamanho da janela e salva em winy e winx
    wmove(win, winy - 2, 1);

    enum tile (*board)[game->width] = game->board;// Aponta para o tabuleiro do jogo
    uint8_t (*blink)[game->width] = game->blink; // Aponta para a matriz de "piscar" das peças ao ganhar

    // Loops pelas linhas e colunas do tabuleiro 
    for (int i = 0; i <game->height; i++) {
        for (int j = 0; j < game->width; j++) {
            // Aplica cor da peça e efeito de piscar se necessário
            wattrset(win, COLOR_PAIR(board[i][j]) | A_BLINK * blink[i][j]);
            waddstr(win, checkers[board[i][j]]);// Desenha a peça
            wattrset(win, COLOR_PAIR(0) | A_NORMAL);

            // Adiciona espaçamento entre as peças (exceto na última coluna)
            if (j <= game->height - 1)
                waddstr(win, "  ");
        }

        getyx(win, cury, curx);

        if (i < game->height - 1) {
            wmove(win, cury - 1, 3);

            for (int j = 0; j < game->width - 1; j++) {
#ifdef C4C_COLOR
                waddch(win, '+'); // Desenha '+' como separador se cores habilitadas
#else
                waddch(win, '.'); // Desenha '.' como separador se cores não habilitadas
#endif
                getyx(win, cury, curx);
                wmove(win, cury, curx + 2);
            }
        }

        wmove(win, cury - 1, 1);
    }
}

/**
  Destaca as peças vencedoras no tabuleiro, marcando-as para piscar.
 
  Percorre a linha de peças a partir da posição fornecida (pos) 
  na direção do eixo vencedor (winning_axis) e ativa o efeito de 
  piscar para cada peça da mesma cor.
 
  Parâmetros:
 *   win          - Janela do curses onde o tabuleiro é desenhado.
 *   game         - Estrutura do jogo contendo o tabuleiro e o estado de piscar.
 *   pos          - Posição inicial da peça vencedora.
 *   winning_axis - Índice da direção vencedora no array `neighbour_pos`.
 */
static void mark_winning_tiles(WINDOW *win, struct game *game, struct position *pos, int winning_axis)
{
    enum tile (*board)[game->width] = game->board;
    uint8_t (*blink)[game->width] = game->blink;

    wattrset(win, COLOR_PAIR(board[pos->y][pos->x] | A_BLINK));

    struct position chk = { pos->x, pos->y };

    while (is_valid_nonempty_pos(game, &chk) &&
            board[pos->y][pos->x] == board[chk.y][chk.x]) {
        blink[chk.y][chk.x] = 1;

        chk.x += neighbour_pos[winning_axis].x;
        chk.y += neighbour_pos[winning_axis].y;
    }
    
    wattrset(win, COLOR_PAIR(0));
}

volatile bool player_wants_to_quit = false;

static void set_player_wants_to_quit(int signo)
{
    player_wants_to_quit = true;
}

/**
  Inicia e gerencia o fluxo completo de uma partida do jogo.
 
  Esta função configura o tabuleiro, inicializa a interface gráfica, 
  gerencia o loop de jogadas, verifica vitórias ou empate,
  atualiza a exibição do tabuleiro e finaliza o jogo.
 
  Parâmetros:
 *   width     - Largura do tabuleiro.
 *   height    - Altura do tabuleiro.
 *   mode      - Modo de jogo (local, contra PC ou rede).
 *   on_redraw - Ponteiro para função de redraw chamada em eventos de resize ou redraw.
 *   ctx       - Contexto passado para a função on_redraw.
 */
void start_game(struct game_params *params, void (*on_redraw)(WINDOW *, void *ctx), void *ctx)
{
    // Armazena a ação anterior do sinal SIGINT para restaurar depois
    static struct sigaction old_act;
    // Configura um handler para SIGINT para permitir que o jogador saia do jo
    sigaction(SIGINT, &(struct sigaction) { .sa_handler = set_player_wants_to_quit }, &old_act);

    redrawwin(stdscr);
    wrefresh(stdscr);

    // Aloca tabuleiro e matriz de blink e armazena na struct game
    struct game game = {
        .width = params->width, .height = params->height,
        .blink = calloc(1, sizeof(uint8_t[params->height][params->width])),
        .board = calloc(1, sizeof(enum tile[params->height][params->width]))
    };

    int win_width = game.width * 3 + 2;
    int win_height = game.height * 2 + 3;

    int x_offset = (COLS - win_width) / 2;
    int y_offset = (LINES - win_height) / 2;
    
     // Cria a janela do jogo com as dimensões e posição calculadas
    WINDOW *game_win = newwin(win_height, win_width, y_offset, x_offset);
    keypad(game_win, TRUE);

    // Inicializa cores para as peças vermelhas e amarelas
    init_pair(TILE_RED_CHECKER, COLOR_RED, COLOR_BLACK);
    init_pair(TILE_YLW_CHECKER, COLOR_YELLOW, COLOR_BLACK);

    box(game_win, 0, 0);

    wmove(game_win, 0, 0);
    wclrtoeol(game_win);

    print_board(game_win, &game);

    game.cur_player = PLAYER_RED;

    /* Se modo é vs IA, inicializa a thread da IA */
    if (params->mode == PLAY_LOCAL_PC) {
        ia_init(&game);
    }

    int n_turns = 0;// Contador de turnos
    enum tile (*board)[game.width] = game.board;

    while (true) {
        print_hud(&game);
        refresh();

        if (n_turns == params->width * params->height) {
            print_board(game_win, &game);
            wrefresh(game_win);

            mvwaddstr(stdscr, LINES - 3, 1, _("Tie!"));// Exibe mensagem de empate
            wgetch(stdscr);

            break;
        }

        // Realiza a jogada e retorna a posição escolhida
        struct position pos;

        if (params->mode == PLAY_LOCAL) {
            /* Modo local: ambos os jogadores são humanos */
            pos = local_play(game_win, &game, on_redraw, ctx);
        } else if (params->mode == PLAY_LOCAL_PC) {
            /* Modo vs IA */
            if (game.cur_player == PLAYER_RED) {
                /* Jogador humano (vermelho) faz sua jogada */
                pos = local_play(game_win, &game, on_redraw, ctx);
            } else {
                /* IA (amarela) faz sua jogada usando a thread */
                int col;
                /* Aguarda até que a IA tenha um movimento pronto */
                while ((col = ia_get_move()) == -1) {
                    usleep(50000);  /* Dorme 50ms para não consumir CPU */
                }
                pos.x = col;
                pos.y = col_is_not_full(&game, pos.x);
            }
        }

        // Atualiza o tabuleiro com a peça do jogador atual
        board[pos.y][pos.x] = PLAYER_TO_CHECKER(game.cur_player);

        print_board(game_win, &game);

        // Verifica se a jogada atual causou vitória
        uint8_t (*winning_axes)[4] = check_win(&game, &pos);

        if (winning_axes) {
            for (int i = 0; i < 4; i++) {
                if ((*winning_axes)[i] >= 3) {
                    // Marca as peças vencedoras na direção do eixo
                    mark_winning_tiles(game_win, &game, &pos, i);
                    mark_winning_tiles(game_win, &game, &pos, i + 4);
                }
            }

            print_board(game_win, &game);
            wrefresh(game_win);

            y_offset = (LINES - 3) / 2 - 10;
            x_offset = (COLS - 20) / 2;

            mvwaddstr(stdscr, LINES - 3, 1, _(end_messages[params->mode][game.cur_player]));
            wgetch(stdscr);

            break;
        }

        n_turns++;// Incrementa contador de turnos
        game.cur_player = !game.cur_player; // Alterna jogador
    }

    delwin(game_win); // Remove janela do jogo

    /* Se foi modo vs IA, encerra a thread da IA */
    if (params->mode == PLAY_LOCAL_PC) {
        ia_end();
    }

     // Liberação de memoria alocada
    free(game.board);
    free(game.blink);
    
     // Restaura configuração original
    sigaction(SIGINT, &old_act, NULL);
    player_wants_to_quit = false;
}
