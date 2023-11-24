#include <stdbool.h>
#include "server.h"

int volatile exit_flag = false;

void sigHandler(int signal) {
    exit_flag = true;
    printf("exit_flag set to %d\n", exit_flag);
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
        {READ, done, CLEANUP},
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

//    signal(SIGINT, sigHandler); //start the thread in SETUP, change the state to END when exit_flag is true

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
    struct server_opts *opts = (struct server_opts *) arg;
    struct sockaddr from_addr;
    socklen_t from_addr_len = sizeof(struct sockaddr_in);
    char buffer[MAX_LEN];
    struct stash window[WIN_SIZE];
    int ret;
    uint32_t client_seq_num;
    uint32_t server_seq_num;

    client_seq_num = 0;
    server_seq_num = 0;

    memset(buffer, 0, MAX_LEN);
    ret = fill_buffer(opts->sock_fd, buffer, &from_addr, &from_addr_len);
    printf("ret: %d", ret);
    if (ret > 0)
    {
        handle_data_in(opts, buffer, &client_seq_num, &server_seq_num, window, &from_addr, &from_addr_len);
    }

    if(opts->msg)
    {
        return error;
    }

    if(exit_flag == true)
    {
        return done;
    }

    return ok;
}

int fill_buffer(int sock_fd, char *buffer,  struct sockaddr *from_addr, socklen_t *from_addr_len)
{
    ssize_t rbytes = recvfrom(sock_fd, buffer, MAX_LEN, 0, from_addr, from_addr_len);
    printf("rbytes: %zd\n", rbytes);

    return 0;
}

void print_packet(struct packet *pkt)
{
    printf("----------Packet Info----------\n");
    printf("Seq Num: %d\n", pkt->header->seq_num);
    printf("Ack Num: %d\n", pkt->header->ack_num);
    printf("Flags: %d\n", pkt->header->flags);
    printf("Data Len: %d\n", pkt->header->data_len);
    printf("Data: %s\n", pkt->data);

}
