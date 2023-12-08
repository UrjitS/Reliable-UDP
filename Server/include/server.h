//
// Created by Colin Lam on 2023-11-13.
//

#ifndef RELIABLE_UDP_SERVER_H
#define RELIABLE_UDP_SERVER_H

#include <arpa/inet.h>
#include <inttypes.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "fsm.h"
#include "helpers.h"

#define SERVER_ARGS 3
#define IP_INDEX 1
#define PORT_INDEX 2
#define MAX_LEN 1024
#define WIN_SIZE 5
#define ACK_SIZE 13
#define ACK_DATA_LEN 2

#define ACK 1

struct stash {
    int cleared; // 0 = cleared, 1 = not cleared
    uint32_t rel_num;
    uint32_t seq_num;
    char *data;
};

struct server_opts
{
    int argc;
    int ip_family;
    int sock_fd;
    FILE *graph_fd;
    FILE *stat_fd;
    in_port_t host_port;
    uint32_t client_seq_num;
    uint32_t server_seq_num;
    time_t start_time;
    char *msg;
    char *host_ip;
    char **argv;
    struct stash window[WIN_SIZE];
};

struct packet_header {
    uint32_t seq_num;      // Sequence Number
    uint32_t ack_num;       // Ack Number
    uint8_t flags;          // 8 bit flags
    uint16_t data_len;      // 16 bit body size
};

struct packet {
    struct packet_header *header;
    char *data;
};

int get_ip_family(const char *ip_addr);
int parse_in_port_t(struct server_opts *opts);
void init_window(struct server_opts *opts);
void init_graphing(struct server_opts *opts);
int set_socket_non_block(struct server_opts *opts);
int fill_buffer(int sock_fd, char *buffer,  struct sockaddr *from_addr, socklen_t *from_addr_len);
void deserialize_packet(char *header, struct packet *pkt);
void handle_data_in(struct server_opts *opts, char *buffer, uint32_t *client_seq_num, uint32_t *server_seq_num
        , struct stash *window, struct sockaddr *from_addr, socklen_t *from_addr_len);
void free_pkt(struct packet *pkt);
void return_ack(int sock_fd, uint32_t *server_seq_num, uint32_t pkt_seq_num,
                struct sockaddr *from_addr, const socklen_t *from_addr_len);
void generate_ack(char *ack, uint32_t server_seq_num, uint32_t pkt_seq_num, uint8_t flags, uint16_t data_len);
void manage_window(uint32_t *client_seq_num, struct stash *window, struct packet *pkt);
void deliver_data(char *data, uint32_t seq_num);
void reset_stash(struct stash *stash);
void order_window(const uint32_t *client_seq_num, struct stash *window);
void check_window(uint32_t *client_seq_num, struct stash *window);
void copy_stash(const struct stash *src, struct stash *dest);
void print_packet(struct packet *pkt);
void print_window(struct stash *window);

#endif
