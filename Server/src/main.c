#include <stdbool.h>
#include "server.h"

int volatile exit_flag = false;

void sigHandler(int signal) {
    exit_flag = true;
}

static int compare_transitions(const void *a, const void *b);
static enum state_codes lookup_transitions(enum state_codes current_state, enum ret_codes return_code);

int (* state[])(void *arg) = {entry_state, parse_args, set_up,
                              do_read, print_error, clean_up, end_state}; //function pointers go here

struct transition state_transitions[] = {
        {ENTRY, ok, PARSEARGS},
        {PARSEARGS, ok, SETUP},
        {PARSEARGS, error, FATALERROR},
        {SETUP, ok, READ},
        {SETUP, error, FATALERROR},
        {READ, ok, READ},
        {READ, error, FATALERROR},
        {FATALERROR, ok, CLEANUP},
        {CLEANUP, ok, END}
};

int main (int argc, char *argv[])
{
    enum state_codes cur_state = ENTRY;
    enum ret_codes rc;
    int (* state_fun)(void *arg);

    struct server_opts opts;
    memset(&opts, 0 , sizeof(struct server_opts));
    opts.argc = argc;
    opts.argv = argv;

    while (1) {
        state_fun = state[cur_state];
        rc = state_fun(&opts);
        if (END == cur_state)
            break;
        cur_state = lookup_transitions(cur_state, rc);
    }

    return EXIT_SUCCESS;
}

/* Comparison function for binary search */
static int compare_transitions(const void *a, const void *b) {
    struct transition *transitionA = (struct transition *)a;
    struct transition *transitionB = (struct transition *)b;

    if (transitionA->src_state == transitionB->src_state) {
        return (int)(transitionA->ret_code) - (int)(transitionB->ret_code);
    } else {
        return (int)(transitionA->src_state) - (int)(transitionB->src_state);
    }
}

/* Function to look up the next state based on current state and return code using binary search */
static enum state_codes lookup_transitions(enum state_codes current_state, enum ret_codes return_code) {
    struct transition key = {current_state, return_code, 0};
    struct transition *result;

    result = (struct transition *)bsearch(&key, state_transitions, sizeof(state_transitions) / sizeof(struct transition),
                                          sizeof(struct transition), compare_transitions);

    if (result != NULL) {
        return result->dst_state;
    } else {
        printf("Invalid transition!\n");
        return END;
    }
}

int do_read(void *arg)
{
    printf("Reading...\n");

    struct server_opts *opts = (struct server_opts *) arg;
    struct sockaddr from_addr;
    socklen_t from_addr_len = sizeof(struct sockaddr_in);
    char header[HEADER_LEN];
    struct stash window[WIN_SIZE];
    ssize_t rbytes;
    uint32_t client_seq_num;
    uint32_t server_seq_num;

    client_seq_num = 0;
    server_seq_num = 0;

    while(1)
    {
        rbytes = recvfrom(opts->sock_fd, header, HEADER_LEN, 0, &from_addr, &from_addr_len);
        if(rbytes == -1)
        {
            if(exit_flag)
            {
                opts->msg= strdup("server closing\n");
            }
            else
            {
                opts->msg = strdup("recvfrom error\n");
            }
            break;
        }

        struct packet pkt;
        deserialize_header(header, &pkt);
        pkt.data = malloc(sizeof(pkt.header.data_len+1));
        rbytes = recvfrom(opts->sock_fd, &pkt.data, pkt.header.data_len, 0,&from_addr, &from_addr_len);
        if(rbytes == -1)
        {
            if(exit_flag)
            {
                opts->msg= strdup("server closing\n");
            }
            else
            {
                opts->msg = strdup("recvfrom error\n");
            }
            break;
        }
        pkt.data[pkt.header.data_len] = '\0';

        if(pkt.header.seq_num < client_seq_num)
        {
            //RETURN ACK
            return_ack(opts->sock_fd, &server_seq_num, pkt.header.seq_num, &from_addr, from_addr_len);
            //IGNORE PACKET
        }
        else if(pkt.header.seq_num <= client_seq_num && pkt.header.seq_num < client_seq_num+WIN_SIZE)
        {
            //RETURN ACK
            return_ack(opts->sock_fd, &server_seq_num, pkt.header.seq_num, &from_addr, from_addr_len);
            //STASH AND DELIVER LOGIC
            manage_window(&client_seq_num, window, &pkt);
        }

        memset(&pkt.header, 0, sizeof(struct packet_header));
        free(pkt.data);
    }

    for(size_t i = 0; i < WIN_SIZE; i++)
    {
        reset_stash(&window[i]);
    }

    if(opts->msg)
    {
        return error;
    }

    return ok;
}
