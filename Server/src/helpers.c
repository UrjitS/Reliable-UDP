//
// Created by Colin Lam on 2023-12-05.
//

#include <unistd.h>
#include <stdio.h>
#include "helpers.h"

void write_to_graph(int graph_fd, uint32_t ack_num)
{
    write(graph_fd, &ack_num, sizeof(uint32_t));
}
void write_to_stat(FILE *stat, uint32_t server_seq_num, uint32_t client_seq_num)
{
    fprintf(stat, "Number of Packets Received from Client: %d\n", client_seq_num);
    fprintf(stat, "Number of Packets Sent by Server: %d\n", server_seq_num);
}

