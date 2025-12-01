/* Header da IA do c4c
 * Define a interface para inicializar, obter movimentos e encerrar a IA
 */

#ifndef GAME_IA_H
#define GAME_IA_H

#include <stdbool.h>

struct game;

/* Inicializa a IA e começa a thread de execução
 * Parâmetro: game - ponteiro para a estrutura do jogo
 * Retorna: true se conseguiu iniciar, false caso contrário
 */
bool ia_init(struct game *game);

/* Obtém o próximo movimento da IA se estiver disponível
 * Retorna: número da coluna (0-6), ou -1 se não há movimento pronto
 */
int ia_get_move(void);

/* Encerra a IA e aguarda a thread terminar */
void ia_end(void);

#endif /* GAME_IA_H */
