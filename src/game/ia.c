/* uma IA para o jogo c4c usando thread e mutex
 * A IA escolhe colunas de forma aleatória para fazer seus movimentos
 * Usa threading para simular um jogador remoto de forma simples
 */

#include <stdlib.h>         // malloc, rand          
#include <pthread.h>        // usando threads e mutex
#include <unistd.h>         // usleep(dormir)
#include <stdbool.h> 

#include "game/ia.h"
#include "game/common.h"

/* armazena o estado da IA */
struct ia_state {
    pthread_mutex_t mutex;    /* Protege o acesso às variáveis compartilhadas */
    int32_t col;              /* Coluna escolhida pela IA (-1 = sem movimento) */
    bool has_move;            /* Indica se há um movimento válido aguardando */
    bool running;             /* Indica se a thread deve continuar viva */
};

/* Variável global que guarda o estado da IA */
static struct ia_state IA = {
    .mutex = PTHREAD_MUTEX_INITIALIZER,
    .col = -1,
    .has_move = false,
    .running = false
};

static bool ia_thread_running = false;
static pthread_t ia_thread;

/* Argumentos passados para a thread da IA */
struct ia_thread_args {
    struct game *game;
};

/* Função que a thread da IA executa
 * Simula o processamento da IA: aguarda um tempo e escolhe uma coluna aleatória
 */
static void *ia_event_loop(void *ctx)
{
    // recebe o jogo como argumento
    struct ia_thread_args *args = (struct ia_thread_args *)ctx;
    struct game *game = args->game;
    free(args);

    // loop principal da thread
    while (true) {
        /* Verifica se deve matar/parar a thread */
        pthread_mutex_lock(&IA.mutex);
        if (!IA.running) {
            pthread_mutex_unlock(&IA.mutex);
            break;
        }
        pthread_mutex_unlock(&IA.mutex);

        /* Aguarda um tempo para simular "pensamento" da IA (500-1500ms) */
        // usleep(500000 + rand() % 1000000);

        /* Encontra uma coluna válida (não cheia) para fazer a jogada */
        pthread_mutex_lock(&IA.mutex);

        int col = -1;
        // tenta por 10 vezes achar algum movimento aleatorio para fazer
        for (int attempt = 0; attempt < 10; attempt++) {
            col = rand() % game->width;
            if (col_is_not_full(game, col) != -1) {
                break;
            }
        }

        /* Se não encontrou em 10 tentativas, procura sequencialmente */
        if (col_is_not_full(game, col) == -1) {
            for (int i = 0; i < game->width; i++) {
                if (col_is_not_full(game, i) != -1) {
                    col = i;
                    break;
                }
            }
        }

        /* Se encontrou uma coluna válida, guarda o resultado */
        if (col_is_not_full(game, col) != -1) {
            // settar o movimento
            IA.col = col;
            // settar que o movimento é valido
            IA.has_move = true;
        }

        pthread_mutex_unlock(&IA.mutex);
    }

    return NULL;
}

/* Inicializa a IA e começa a thread
 * Parâmetro: game - ponteiro para a estrutura do jogo
 * Retorna: true se conseguiu iniciar, false caso contrário
 */
bool ia_init(struct game *game)
{
    // se já está rodando, não inicia
    if (ia_thread_running)
        return false;

    struct ia_thread_args *args = malloc(sizeof *args);
    if (!args)
        return false;

    args->game = game;

    /* Inicia a thread da IA */
    if (pthread_create(&ia_thread, NULL, ia_event_loop, args) != 0) {
        free(args);
        return false; // não foi possivel iniciar a thread
    }

    /* Marca que a IA está rodando */
    pthread_mutex_lock(&IA.mutex);
    IA.running = true;
    IA.has_move = false;
    IA.col = -1;
    pthread_mutex_unlock(&IA.mutex);

    ia_thread_running = true;
    return true;
}

/* Obtém o movimento da IA se houver um disponível
 * Retorna: número da coluna (0-6), ou -1 se não há movimento pronto
 */
int ia_get_move(void)
{
    pthread_mutex_lock(&IA.mutex);

    int resp = -1;

    /* Verifica se há um movimento válido para ser consumido */
    if (IA.has_move == true && IA.col >= 0) {
        resp = IA.col;

        /* Consome o movimento */
        IA.col = -1;
        IA.has_move = false;
    }

    pthread_mutex_unlock(&IA.mutex);

    return resp;
}

/* Encerra a IA e aguarda a thread terminar */
void ia_end(void)
{
    if (!ia_thread_running)
        return;

    /* Sinaliza para a thread parar */
    pthread_mutex_lock(&IA.mutex);
    IA.running = false;
    pthread_mutex_unlock(&IA.mutex);

    /* Aguarda a thread terminar */
    pthread_join(ia_thread, NULL);

    ia_thread_running = false;
}
