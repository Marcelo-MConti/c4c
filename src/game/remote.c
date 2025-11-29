#include <stdint.h>
#include <unistd.h>
#include <pthread.h>

#include <sys/socket.h>

#include <arpa/inet.h>
#include <netinet/in.h>

#include "util.h"

#include "game/remote.h"

enum net_msg_type {
    HANDSHAKE,
    MOVE,
    ACK
};

struct net_msg {
    uint32_t seq_id;
    uint32_t type;
    union {
        struct {
            uint32_t rand;
        } handshake;
        struct {
            uint32_t col;
        } move;
        struct {
            uint32_t ref;
        } ack;
    };
} __attribute__((packed));

struct net_msg hanshake(const char *addr, int port)
{
    struct net_msg msg = {
        .seq_id = 0/**/,
        .type = HANDSHAKE,
        .handshake = { rand32() }
    };

    struct sockaddr_in net_sock = {.sin_family = AF_INET6, .sin_port = htons(port)};

    // XXX: use `getaddrinfo` com a opção `AI_V4MAPPED` para
    // suportar IPv4 + IPv6 + nome de domínio automaticamente
    if(inet_pton(AF_INET6, addr, &(net_sock.sin_addr)) <= 0) {
        // msg.type = ERROR;
        return msg;
    }

    int net_file;
    if ((net_file = socket(AF_INET6, SOCK_DGRAM, 0)) < 0) {
        // msg.type = ERROR;
        return msg;
    }

    if (sendto(net_file, &msg, sizeof(msg), 0, (const struct sockaddr*) &net_sock, sizeof(net_sock)) < 0) {
        close(net_file);
        // msg.type = ERROR;
        return msg;
    }
  
  
    return msg;
}
