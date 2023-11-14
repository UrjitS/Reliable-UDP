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
#include <sys/socket.h>
#include <sys/types.h>

#include "fsm.h"

#define SERVER_ARGS 3
#define IP_INDEX 1
#define PORT_INDEX 2


struct server_opts
{
    int argc;
    char **argv;
    in_port_t host_port;
    char *host_ip;
    int ip_family;
    int sock_fd;
    char *msg;
};

struct packet_header {
    uint32_t seq_numb;      // Sequence Number
    uint32_t ack_num;       // Ack Number
    uint8_t flags;          // 8 bit flags
    uint16_t data_len;      // 16 bit body size
};

int get_ip_family(const char *ip_addr);
int parse_in_port_t(struct server_opts *opts);

#endif
