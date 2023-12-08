//
// Created by Colin Lam on 2023-12-05.
//

#include "helpers.h"

void write_to_graph(FILE *graph, uint32_t ack_num, time_t start_time)
{
    time_t now = time(0);
    now = start_time - now;
    fprintf(graph, "%d, %ld\n", ack_num, now);
}
void write_to_stat(FILE *stat, uint32_t server_seq_num, uint32_t client_seq_num)
{
    fprintf(stat, "Number of Packets Received from Client: %d\n", client_seq_num);
    fprintf(stat, "Number of Packets Sent by Server: %d\n", server_seq_num);
}

