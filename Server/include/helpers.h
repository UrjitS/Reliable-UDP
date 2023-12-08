//
// Created by Colin Lam on 2023-12-05.
//

#ifndef RELIABLE_UDP_HELPERS_H
#define RELIABLE_UDP_HELPERS_H

#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>

void write_to_graph(FILE *graph, uint32_t ack_num, time_t start_time);
void write_to_stat(FILE *stat, uint32_t server_seq_num, uint32_t client_seq_num);

#endif //RELIABLE_UDP_HELPERS_H
