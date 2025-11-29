#include <stdint.h>
#include <stdbool.h>

#include "game.h"

enum net_msg_type {
    HANDSHAKE,
    MOVE,
    ACK
};

bool remote_init_play(char *host, uint16_t port);
int remote_send_move(move_t col);
move_t remote_get_move(void);
bool remote_reinit_play(void);
void remote_end(void);
