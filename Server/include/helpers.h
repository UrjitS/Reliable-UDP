//
// Created by Colin Lam on 2023-12-05.
//

#ifndef RELIABLE_UDP_HELPERS_H
#define RELIABLE_UDP_HELPERS_H



void write_to_graph(int graph_fd, uint32_t ack_num);
void write_to_stat(FILE *stat, uint32_t server_seq_num, uint32_t client_seq_num);

#endif //RELIABLE_UDP_HELPERS_H
