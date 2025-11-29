#include <stdint.h>
#include <stdbool.h>

bool remote_init_play(char *host, uint16_t port);
int remote_send_move(int col);
int remote_get_move(void);
bool remote_reinit_play(void);
void remote_end(void);
