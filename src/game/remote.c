#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdint.h>
#include <sys/socket.h>
#include <pthread.h>
#include <unistd.h>

#include "util.h"
#include "game/remote.h"

typedef struct net_msg {
  uint32_t seq_id;
  enum net_type type;
  uint32_t number;
} net_msg;

struct net_msg hanshake(const char *addr, int port)
{
  net_msg msg = {HANSHAKE, rand32()};

  struct sockaddr_in net_sock = {.sin_family = AF_INET6, .sin_port = htons(port)};

  if(inet_pton(AF_INET6, addr, &(net_sock.sin_addr)) <= 0) {
    msg.type = ERROR;
    return msg;
  }

  int net_file;
  if ((net_file = socket(AF_INET6, SOCK_DGRAM, 0)) < 0) {
    msg.type = ERROR;
    return msg;
  }

  if (sendto(net_file, &msg, sizeof(msg), 0, (const struct sockaddr*) &net_sock, sizeof(net_sock)) < 0) {
    close(net_file);
    msg.type = ERROR;
    return msg;
  }
  
  
  return msg;
}
