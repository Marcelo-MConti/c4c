#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

#include <netdb.h>

#include <sys/types.h>
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

struct net_thread_args {
    int sock_fd;
};

// meu mutex global
static pthread_mutex_t net_state_mutex = PTHREAD_MUTEX_INITIALIZER;

// variavel global que guarda meu ultimo movimento
// inicializada com "-1"
static int32_t col = -1;

// variavel global, que guarde se na região critica tem uma jogada remota
static bool remote = false;

// função auxiliar para enviar um ACK (adotando que o socket que já foi conectado)
static void send_ACK(int sock_fd, uint32_t ref_seq)
{
    // montar meu ack
    struct net_msg ack = {
        .seq_id = ref_seq,      // ecoar a sequencia recebida (confirmada)
        .type = ACK,
        .ack = {ref_seq}
    };

    // adotando que ja ocorreu a conexão com o socket
    ssize_t s = send(sock_fd, &(ack), sizeof(ack), 0);

    if(s < 0){
        print("deu ruim no ACK");
        return;
    }
}


// falta colocar o loop da thread na rede
static void *event_loop(void *ctx)
{
    struct net_thread_args *args = ctx;
    if(!args) return NULL;

    int sock_fd = args->sock_fd;
    free(args);

    struct net_msg msg;


    free(args);
    return NULL;
}

bool thread_running = false;
pthread_t net_thread;

bool remote_init_play(char *host, uint16_t port)
{
    if (thread_running)
        return false;

    struct addrinfo hints = {
        .ai_flags = AI_V4MAPPED | AI_ADDRCONFIG,
        .ai_socktype = SOCK_DGRAM
    };

    int sock_fd = -1; 
    struct addrinfo *info;

    if (getaddrinfo(host, NULL, &hints, &info) != 0)
       return false;

    for (struct addrinfo *cur = info; cur; cur = cur->ai_next) {
        sock_fd = socket(cur->ai_family, SOCK_DGRAM, cur->ai_protocol);

        if (sock_fd == -1)
            continue;
        
        switch (cur->ai_family) {
            case AF_INET: ;
                struct sockaddr_in *sin = (struct sockaddr_in *)cur->ai_addr;
                sin->sin_port = htons(port);

                break;
            case AF_INET6: ;
                struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)cur->ai_addr;
                sin6->sin6_port = htons(port);

                break;
            default:
                close(sock_fd);
                sock_fd = -1;

                continue;
        }

        if (connect(sock_fd, cur->ai_addr, cur->ai_addrlen) != -1)
            break;

        close(sock_fd);
        sock_fd = -1;
    }

    if (sock_fd == -1)
        goto fail_addrinfo;

    struct net_thread_args *args = malloc(sizeof *args);
    args->sock_fd = sock_fd;
    
    if (pthread_create(&net_thread, NULL, event_loop, args) != 0)
        goto fail_thread_args;

    thread_running = true;
    return true;

fail_thread_args:
    free(args);
fail_addrinfo:
    freeaddrinfo(info);

    return false;
}

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

// -1 : Não houve movimento ainda
//  x : numero da coluna
int remote_get_move(void)
{
    // entro na região critica
    pthread_mutex_lock(&net_state_mutex);
    int resp = -1;

    // verifico se tem uma jogada valida para ser consumida
    if(col >= 0 && remote == true){
        // guardo a resposta
        resp = col;

        // consumir
        col = -1;
        remote = false;
    }

    // saio da região critica
    pthread_mutex_lock(&net_state_mutex);

    // retorno a resposta
    return resp;
}

// reenviar o handshake e permitir jogar novamente
bool remote_reinit_play()
{

}

// matar a thread
bool remote_end(){

}